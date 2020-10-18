/*****************************************************************************
    NumeRe: Framework fuer Numerische Rechnungen
    Copyright (C) 2020  Erik Haenel et al.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef FILTERING_HPP
#define FILTERING_HPP

#include <utility>
#include <vector>
#include <cmath>
#include "../io/file.hpp"
#include "../utils/tools.hpp"

//void showMatrix(const vector<vector<double> >&);

namespace NumeRe
{
    /////////////////////////////////////////////////
    /// \brief This structure contains the necessary
    /// information to create an instance of one of
    /// the following filters.
    /////////////////////////////////////////////////
    struct FilterSettings
    {
        enum FilterType
        {
            FILTER_NONE,
            FILTER_WEIGHTED_LINEAR,
            FILTER_GAUSSIAN,
            FILTER_SAVITZKY_GOLAY
        };

        FilterType type;
        size_t row;
        size_t col;
        double alpha;

        FilterSettings(FilterType _type = FILTER_NONE, size_t _row = 1u, size_t _col = 1u, double _alpha = NAN) : type(_type), row(std::max(_row, 1u)), col(std::max(_col, 1u)), alpha(_alpha)
        {
            //
        }
    };


    /////////////////////////////////////////////////
    /// \brief This function is a simple helper to
    /// implement a power of two.
    ///
    /// \param val double
    /// \return double
    ///
    /////////////////////////////////////////////////
    inline double pow2(double val)
    {
        return val * val;
    }


    /////////////////////////////////////////////////
    /// \brief This is an abstract base class for any
    /// type of a data filter. Requires some methods
    /// to be implemented by its child classes.
    /////////////////////////////////////////////////
    class Filter
    {
        protected:
            FilterSettings::FilterType m_type;
            std::pair<size_t, size_t> m_windowSize;
            bool m_isConvolution;

        public:
            /////////////////////////////////////////////////
            /// \brief Filter base constructor. Will set the
            /// used window sizes.
            ///
            /// \param row
            /// \param col
            ///
            /////////////////////////////////////////////////
            Filter(size_t row, size_t col) : m_type(FilterSettings::FILTER_NONE), m_isConvolution(false)
            {
                // This expression avoids that someone tries to
                // create a filter with a zero dimension.
                m_windowSize = std::make_pair(max(row, 1u), max(col, 1u));
            }

            /////////////////////////////////////////////////
            /// \brief Empty virtual abstract destructor.
            /////////////////////////////////////////////////
            virtual ~Filter() {}

            /////////////////////////////////////////////////
            /// \brief Virtual operator() override. Has to be
            /// implemented in the child classes and shall
            /// return the kernel value at position (i,j).
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            virtual double operator()(size_t i, size_t j) const = 0;

            /////////////////////////////////////////////////
            /// \brief Virtual method for applying the filter
            /// to a distinct value. Has to be implemented in
            /// all child classes.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \param val double
            /// \return double
            ///
            /////////////////////////////////////////////////
            virtual double apply(size_t i, size_t j, double val) const = 0;

            /////////////////////////////////////////////////
            /// \brief This method returns, whether the
            /// current filter is a convolution, ie. whether
            /// the returned value may be used directly or
            /// if all values of a window have to be
            /// accumulated first.
            ///
            /// \return bool
            ///
            /////////////////////////////////////////////////
            bool isConvolution() const
            {
                return m_isConvolution;
            }

            /////////////////////////////////////////////////
            /// \brief This method returns the type of the
            /// current filter as a value of the FilterType
            /// enumeration.
            ///
            /// \return FilterType
            ///
            /////////////////////////////////////////////////
            FilterSettings::FilterType getType() const
            {
                return m_type;
            }

            /////////////////////////////////////////////////
            /// \brief This method returns the window size of
            /// the current filter as a std::pair in the
            /// order (row,col).
            ///
            /// \return std::pair<size_t,size_t>
            ///
            /////////////////////////////////////////////////
            std::pair<size_t,size_t> getWindowSize() const
            {
                return m_windowSize;
            }
    };


    /////////////////////////////////////////////////
    /// \brief This class implements a weighted
    /// linear smoothing filter, which applies
    /// something like a "convergent sliding average"
    /// filter.
    /////////////////////////////////////////////////
    class WeightedLinearFilter : public Filter
    {
        private:
            std::vector<double> m_left;
            std::vector<double> m_right;
            std::vector<double> m_top;
            std::vector<double> m_bottom;
            bool is2D;
            std::vector<std::vector<double> > m_filterKernel;

            /////////////////////////////////////////////////
            /// \brief This method will create the filter's
            /// kernel for the selected window size.
            ///
            /// \return void
            ///
            /////////////////////////////////////////////////
            void createKernel()
            {
                m_filterKernel = std::vector<std::vector<double> >(m_windowSize.first, std::vector<double>(m_windowSize.second, 0.0));

                if (!is2D)
                    is2D = m_windowSize.first > 1 && m_windowSize.second > 1;

                double mean_row = m_windowSize.first > 1 ? 0.5 : 0.0;
                double mean_col = m_windowSize.second > 1 ? 0.5 : 0.0;

                // Calculate the filter values
                for (size_t i = 0; i < m_windowSize.first; i++)
                {
                    for (size_t j = 0; j < m_windowSize.second; j++)
                    {
                        if (sqrt(pow2(i/((double)max(1u, m_windowSize.first-1))-mean_row) + pow2(j/((double)max(1u, m_windowSize.second-1))-mean_col)) <= 0.5)
                            m_filterKernel[i][j] = fabs(sqrt(pow2(i/((double)max(1u, m_windowSize.first-1))-mean_row) + pow2(j/((double)max(1u, m_windowSize.second-1))-mean_col)) - 0.5);
                    }
                }
            }

            /////////////////////////////////////////////////
            /// \brief This method will return the correct
            /// value for the left interval boundary.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            double left(size_t i, size_t j) const
            {
                if (!is2D)
                    return validize(m_left.front());

                return validize(m_left[i+1]);
            }

            /////////////////////////////////////////////////
            /// \brief This method will return the correct
            /// value for the right interval boundary.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            double right(size_t i, size_t j) const
            {
                if (!is2D)
                    return validize(m_right.front());

                return validize(m_right[i+1]);
            }

            /////////////////////////////////////////////////
            /// \brief This method will return the correct
            /// value for the top interval boundary.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            double top(size_t i, size_t j) const
            {
                if (!is2D)
                    return 0.0;

                return validize(m_top[j+1]);
            }

            /////////////////////////////////////////////////
            /// \brief This method will return the correct
            /// value for the bottom interval boundary.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            double bottom(size_t i, size_t j) const
            {
                if (!is2D)
                    return 0.0;

                return validize(m_bottom[j+1]);
            }

            /////////////////////////////////////////////////
            /// \brief This method will return the correct
            /// value for the topleft diagonal interval
            /// boundary.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            double topleft(size_t i, size_t j) const
            {
                if (i >= j)
                    return validize(m_left[i-j]);

                return validize(m_top[j-i]);
            }

            /////////////////////////////////////////////////
            /// \brief This method will return the correct
            /// value for the topright diagonal interval
            /// boundary.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            double topright(size_t i, size_t j) const
            {
                if (i+j <= m_windowSize.second-1)
                    return validize(m_top[i+j+2]); // size(m_top) + 2 == m_order

                return validize(m_right[i+j-m_windowSize.second+1]);
            }

            /////////////////////////////////////////////////
            /// \brief This method will return the correct
            /// value for the bottomleft diagonal interval
            /// boundary.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            double bottomleft(size_t i, size_t j) const
            {
                if (i+j <= m_windowSize.first-1)
                    return validize(m_left[i+j+2]); // size(m_left) + 2 == m_order

                return validize(m_bottom[i+j-m_windowSize.first+1]);
            }

            /////////////////////////////////////////////////
            /// \brief This method will return the correct
            /// value for the bottomright diagonal interval
            /// boundary.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            double bottomright(size_t i, size_t j) const
            {
                if (i >= j)
                    return validize(m_bottom[m_windowSize.second-(i-j)+1]); // size(m_bottom) + 2 == m_order

                return validize(m_right[m_windowSize.first-(j-i)+1]);
            }

        protected:
            double m_fallback;
            bool m_invertedKernel;

            /////////////////////////////////////////////////
            /// \brief This method checks, whether the passed
            /// value is a valid value and returns it. If it
            /// is not, it will be replaced by the fallback
            /// value.
            ///
            /// \param val double
            /// \return double
            ///
            /////////////////////////////////////////////////
            double validize(double val) const
            {
                if (isnan(val))
                    return m_fallback;

                return val;
            }

        public:
            /////////////////////////////////////////////////
            /// \brief Filter constructor. Will automatically
            /// create the filter kernel.
            ///
            /// \param row
            /// \param col
            /// \param force2D
            ///
            /////////////////////////////////////////////////
            WeightedLinearFilter(size_t row, size_t col, bool force2D = false) : Filter(row, col)
            {
                m_type = FilterSettings::FILTER_WEIGHTED_LINEAR;
                m_isConvolution = false;
                m_fallback = 0.0;
                m_invertedKernel = false;
                is2D = force2D;

                createKernel();
            }

            /////////////////////////////////////////////////
            /// \brief Filter destructor. Will clear the
            /// previously calculated filter kernel.
            /////////////////////////////////////////////////
            virtual ~WeightedLinearFilter() override
            {
                m_filterKernel.clear();
            }

            /////////////////////////////////////////////////
            /// \brief Override for the operator(). Returns
            /// the filter kernel at the desired position.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            virtual double operator()(size_t i, size_t j) const override
            {
                if (i >= m_windowSize.first || j >= m_windowSize.second)
                    return NAN;

                return m_filterKernel[i][j];
            }

            /////////////////////////////////////////////////
            /// \brief Override for the abstract apply method
            /// of the base class. Applies the filter to the
            /// value at the selected position and returns
            /// the new value.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \param val double
            /// \return virtual double
            ///
            /////////////////////////////////////////////////
            virtual double apply(size_t i, size_t j, double val) const override
            {
                if (i >= m_windowSize.first || j >= m_windowSize.second)
                    return NAN;

                if (!is2D)
                {
                    if (m_invertedKernel)
                        return m_filterKernel[i][j]*val + (1-m_filterKernel[i][j])*(left(i,j) + (right(i,j) - left(i,j))/(m_windowSize.first + 1.0)*(i + 1.0));

                    return (1-m_filterKernel[i][j])*val + m_filterKernel[i][j]*(left(i,j) + (right(i,j) - left(i,j))/(m_windowSize.first + 1.0)*(i + 1.0));
                }

                // cross hair: summarize the linearily interpolated values along the rows and cols at the desired position
                // Summarize implies that the value is not averaged yet
                double dAverage = top(i,j) + (bottom(i,j) - top(i,j)) / (m_windowSize.first + 1.0) * (i+1.0)
                                  + left(i,j) + (right(i,j) - left(i,j)) / (m_windowSize.second + 1.0) * (j+1.0);

                // Additional weighting because are the nearest neighbours
                dAverage *= 2.0;

                // Calculate along columns
                // Find the diagonal neighbours and interpolate the value
                if (i >= j)
                    dAverage += topleft(i,j) + (bottomright(i,j) - topleft(i,j)) / (m_windowSize.second - fabs(i-j) + 1.0) * (j+1.0);
                else
                    dAverage += topleft(i,j) + (bottomright(i,j) - topleft(i,j)) / (m_windowSize.first - fabs(i-j) + 1.0) * (i+1.0);

                // calculate along rows
                // Find the diagonal neighbours and interpolate the value
                if (i + j <= m_windowSize.first + 1)
                    dAverage += bottomleft(i,j) + (topright(i,j) - bottomleft(i,j)) / (i+j+2.0) * (j+1.0);
                else
                    dAverage += bottomleft(i,j) + (topright(i,j) - bottomleft(i,j)) / (m_windowSize.first + m_windowSize.second - i - j) * (double)(m_windowSize.first-i);

                // Restore the desired average
                dAverage /= 6.0;

                if (m_invertedKernel)
                    return (1-m_filterKernel[i][j])*dAverage + m_filterKernel[i][j]*val;

                return (1-m_filterKernel[i][j])*val + m_filterKernel[i][j]*dAverage;
            }

            /////////////////////////////////////////////////
            /// \brief This method is used to update the
            /// internal filter boundaries.
            ///
            /// \return void
            ///
            /////////////////////////////////////////////////
            void setBoundaries(const std::vector<double>& left, const std::vector<double>& right, const std::vector<double>& top = std::vector<double>(), const std::vector<double>& bottom = std::vector<double>())
            {
                m_left = left;
                m_right = right;
                m_top = top;
                m_bottom = bottom;
            }
    };


    /////////////////////////////////////////////////
    /// \brief This class implements a gaussian
    /// smoothing or blurring filter.
    /////////////////////////////////////////////////
    class GaussianFilter : public Filter
    {
        private:
            std::vector<std::vector<double> > m_filterKernel;

            /////////////////////////////////////////////////
            /// \brief This method will create the filter's
            /// kernel for the selected window size.
            ///
            /// \return void
            ///
            /////////////////////////////////////////////////
            void createKernel(double sigma)
            {
                m_filterKernel = std::vector<std::vector<double> >(m_windowSize.first, std::vector<double>(m_windowSize.second, 0.0));

                double sum = 0.0;
                double mean_row = (m_windowSize.first - 1) * 0.5;
                double mean_col = (m_windowSize.second - 1) * 0.5;

                for (size_t i = 0; i < m_windowSize.first; i++)
                {
                    for (size_t j = 0; j < m_windowSize.second; j++)
                    {
                        m_filterKernel[i][j] = exp(-(pow2(i-mean_row) + pow2(j-mean_col)) / (2*pow2(sigma))) / (2*M_PI*pow2(sigma));
                        sum += m_filterKernel[i][j];
                    }
                }

                for (size_t i = 0; i < m_windowSize.first; i++)
                {
                    for (size_t j = 0; j < m_windowSize.second; j++)
                    {
                        m_filterKernel[i][j] /= sum;
                    }
                }
            }

        public:
            /////////////////////////////////////////////////
            /// \brief Filter constructor. Will automatically
            /// create the filter kernel.
            ///
            /// \param row
            /// \param col
            ///
            /////////////////////////////////////////////////
            GaussianFilter(size_t row, size_t col, double sigma) : Filter(row, col)
            {
                m_type = FilterSettings::FILTER_GAUSSIAN;
                m_isConvolution = true;

                createKernel(sigma);
            }

            /////////////////////////////////////////////////
            /// \brief Filter destructor. Will clear the
            /// previously calculated filter kernel.
            /////////////////////////////////////////////////
            virtual ~GaussianFilter() override
            {
                m_filterKernel.clear();
            }

            /////////////////////////////////////////////////
            /// \brief Override for the operator(). Returns
            /// the filter kernel at the desired position.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            virtual double operator()(size_t i, size_t j) const override
            {
                if (i >= m_windowSize.first || j >= m_windowSize.second)
                    return NAN;

                return m_filterKernel[i][j];
            }

            /////////////////////////////////////////////////
            /// \brief Override for the abstract apply method
            /// of the base class. Applies the filter to the
            /// value at the selected position and returns
            /// the new value.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \param val double
            /// \return virtual double
            ///
            /////////////////////////////////////////////////
            virtual double apply(size_t i, size_t j, double val) const override
            {
                if (i >= m_windowSize.first || j >= m_windowSize.second)
                    return NAN;

                if (isnan(val))
                    return 0.0;

                // Gaussian is symmetric, therefore the convolution does not
                // need to be inverted
                return m_filterKernel[i][j] * val;
            }
    };


    /////////////////////////////////////////////////
    /// \brief This class implements a Savitzky-Golay
    /// filter, which is a polynomial smoothing
    /// filter.
    ///
    /// \todo This implementation currently only
    /// supports one-dimensional data sets, because
    /// multi-dimensional ones would require that all
    /// coefficients are stored into data sets (too
    /// difficult to calculate) and to obtain a single
    /// filtered value a matrix multiplication between
    /// the coefficient matrix and the (window sized)
    /// data matrix is required.
    /////////////////////////////////////////////////
    class SavitzkyGolayFilter : public Filter
    {
        private:
            std::vector<std::vector<double>> m_filterKernel;

            /////////////////////////////////////////////////
            /// \brief This private member function finds the
            /// column, which either fits perfectly or is the
            /// nearest possibility to the selected window
            /// size.
            ///
            /// \param _view const FileView&
            /// \return long long int
            ///
            /////////////////////////////////////////////////
            long long int findColumn(const FileView& _view)
            {
                std::vector<size_t> vWindowSizes;

                // Decode all contained window sizes
                for (long long int j = 0; j < _view.getCols(); j++)
                {
                    vWindowSizes.push_back(StrToInt(_view.getColumnHead(j).substr(0, _view.getColumnHead(j).find('x'))));
                }

                // Window size is already smaller than the first
                // available window size
                if (m_windowSize.first < vWindowSizes.front())
                {
                    m_windowSize.first = vWindowSizes.front();
                    m_windowSize.second = m_windowSize.first;
                    m_filterKernel = std::vector<std::vector<double> >(m_windowSize.first, std::vector<double>(m_windowSize.second, 0.0));

                    return 0;
                }

                for (long long int j = 0; j < vWindowSizes.size(); j++)
                {
                    // Found a perfect match?
                    if (m_windowSize.first == vWindowSizes[j])
                    {
                        m_windowSize.second = m_windowSize.first;
                        m_filterKernel = std::vector<std::vector<double> >(m_windowSize.first, std::vector<double>(m_windowSize.second, 0.0));

                        return j;
                    }

                    // Is there a nearest match? (We assume
                    // ascending order of the window sizes)
                    if (m_windowSize.first > vWindowSizes[j] && j+1 < vWindowSizes.size() && m_windowSize.first < vWindowSizes[j+1])
                    {
                        // If the following fits better, increment the index
                        if (m_windowSize.first - vWindowSizes[j] > vWindowSizes[j+1] - m_windowSize.first)
                            j++;

                        m_windowSize.first = vWindowSizes[j];
                        m_windowSize.second = m_windowSize.first;
                        m_filterKernel = std::vector<std::vector<double> >(m_windowSize.first, std::vector<double>(m_windowSize.second, 0.0));

                        return j;
                    }
                    else if (m_windowSize.first > vWindowSizes[j] && j+1 == vWindowSizes.size())
                    {
                        m_windowSize.first = vWindowSizes[j];
                        m_windowSize.second = m_windowSize.first;
                        m_filterKernel = std::vector<std::vector<double> >(m_windowSize.first, std::vector<double>(m_windowSize.second, 0.0));

                        return j;
                    }
                }

                return 0;
            }

            /////////////////////////////////////////////////
            /// \brief This method will create the filter's
            /// kernel for the selected window size.
            ///
            /// \return void
            ///
            /////////////////////////////////////////////////
            void createKernel()
            {
                // Ensure odd numbers
                if (!(m_windowSize.first % 2))
                    m_windowSize.first++;

                // Create a 2D kernel
                if (m_windowSize.second > 1)
                {
                    // Create a file instance
                    GenericFile<double>* _file = getFileByType("<>/params/savitzky_golay_coeffs_2D.dat");

                    if (!_file)
                        return;

                    // Read the contents and assign it to a view
                    try
                    {
                        _file->read();
                    }
                    catch (...)
                    {
                        delete _file;
                        throw;
                    }

                    FileView _view(_file);

                    // Find the possible window sizes
                    long long int j = findColumn(_view);

                    // First element in the column is the
                    // central element in the matrix
                    m_filterKernel[m_windowSize.first/2][m_windowSize.first/2] = _view.getElement(m_windowSize.first*m_windowSize.first/2, j);

                    for (long long int i = 0; i < m_windowSize.first*m_windowSize.first/2; i++)
                    {
                        // left part
                        m_filterKernel[m_windowSize.first - 1 - i % m_windowSize.first][i / m_windowSize.first] = _view.getElement(i, j);
                        // right part
                        m_filterKernel[i % m_windowSize.first][m_windowSize.first - i / m_windowSize.first - 1] = _view.getElement(i, j);
                        // middle column
                    }

                    delete _file;
                    return;
                }

                // Create a 1D kernel
                m_filterKernel = std::vector<std::vector<double> >(m_windowSize.first, std::vector<double>(m_windowSize.second, 0.0));

                for (size_t i = 0; i < m_windowSize.first; i++)
                {
                    m_filterKernel[i][0] = (3.0*pow2(m_windowSize.first) - 7.0 - 20.0*pow2((int)i - (int)m_windowSize.first/2)) / 4.0 / (m_windowSize.first * (pow2(m_windowSize.first) - 4.0) / 3.0);
                }
            }

        public:
            /////////////////////////////////////////////////
            /// \brief Filter constructor. Will automatically
            /// create the filter kernel.
            ///
            /// \param row
            /// \param col
            ///
            /////////////////////////////////////////////////
            SavitzkyGolayFilter(size_t row, size_t col) : Filter(row, col)
            {
                m_type = FilterSettings::FILTER_SAVITZKY_GOLAY;
                m_isConvolution = true;

                createKernel();
            }

            /////////////////////////////////////////////////
            /// \brief Filter destructor. Will clear the
            /// previously calculated filter kernel.
            /////////////////////////////////////////////////
            virtual ~SavitzkyGolayFilter() override
            {
                m_filterKernel.clear();
            }

            /////////////////////////////////////////////////
            /// \brief Override for the operator(). Returns
            /// the filter kernel at the desired position.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \return double
            ///
            /////////////////////////////////////////////////
            virtual double operator()(size_t i, size_t j) const override
            {
                if (i >= m_windowSize.first || j >= m_windowSize.second)
                    return NAN;

                return m_filterKernel[i][j];
            }

            /////////////////////////////////////////////////
            /// \brief Override for the abstract apply method
            /// of the base class. Applies the filter to the
            /// value at the selected position and returns
            /// the new value.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \param val double
            /// \return virtual double
            ///
            /////////////////////////////////////////////////
            virtual double apply(size_t i, size_t j, double val) const override
            {
                if (i >= m_windowSize.first || j >= m_windowSize.second)
                    return NAN;

                if (isnan(val))
                    return 0.0;

                return m_filterKernel[i][j] * val;
            }
    };


    /////////////////////////////////////////////////
    /// \brief This function creates an instance of
    /// the filter specified by the passed
    /// FilterSettings structure.
    ///
    /// The Filter will be created on the heap. The
    /// calling function is responsible for freeing
    /// its memory.
    ///
    /// \param _settings const FilterSettings&
    /// \return Filter*
    ///
    /////////////////////////////////////////////////
    inline Filter* createFilter(const FilterSettings& _settings)
    {
        switch (_settings.type)
        {
            case FilterSettings::FILTER_NONE:
                return nullptr;
            case FilterSettings::FILTER_WEIGHTED_LINEAR:
                return new WeightedLinearFilter(_settings.row, _settings.col);
            case FilterSettings::FILTER_GAUSSIAN:
                return new GaussianFilter(_settings.row, _settings.col, (std::max(_settings.row, _settings.col)-1)/(2*_settings.alpha));
            case FilterSettings::FILTER_SAVITZKY_GOLAY:
                return new SavitzkyGolayFilter(_settings.row, _settings.col);
        }

        return nullptr;
    }


    /////////////////////////////////////////////////
    /// \brief This class is a specialized
    /// WeightedLinearFilter used to retouch missing
    /// data values.
    /////////////////////////////////////////////////
    class RetouchRegion : public WeightedLinearFilter
    {
        public:
            /////////////////////////////////////////////////
            /// \brief Filter constructor. Will automatically
            /// create the filter kernel.
            ///
            /// \param _row
            /// \param _col
            ///
            /////////////////////////////////////////////////
            RetouchRegion(size_t _row, size_t _col, double _dMedian) : WeightedLinearFilter(_row, _col, true) { m_fallback = _dMedian; m_invertedKernel = true;}

            /////////////////////////////////////////////////
            /// \brief This method is a wrapper to retouch
            /// only invalid values. The default value of
            /// invalid values is the median value declared
            /// at construction time.
            ///
            /// \param i size_t
            /// \param j size_t
            /// \param val double
            /// \param med double
            /// \return double
            ///
            /////////////////////////////////////////////////
            double retouch(size_t i, size_t j, double val, double med)
            {
                if (isnan(val) && !isnan(med))
                    return 0.5*(apply(i, j, m_fallback) + med);
                else if (isnan(val) && isnan(med))
                    return apply(i, j, m_fallback);

                return val;
            }
    };
}



#endif // FILTERING_HPP

