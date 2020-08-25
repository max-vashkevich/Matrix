//
//  matrix.hpp
//  Matrix
//
//  Created by Maxim Vashkevich on 7/31/20.
//  Copyright © 2020 Maxim Vashkevich. All rights reserved.
//

// Вся реализация размещена в hpp файле т.к. данный класс — шаблонный

/*
Условные обозначения:
  1. Префикс PMem_ - приватный член класса (PMem = Private Member).
  2. DM - Динамическая матрица (не класс, просто 2 указателя на T).
  3. defined(__GNUC__) - Если используется компилятор GNU
  4. #ifdef _MSC_VER - Если используется компилятор Microsoft
  5. !defined(__APPLE__) - Если используется не MacOS (В Xcode стоит GNU,
     однако он не знвет о файлах c++config
     и functexcept) (используется в свяске с defined(__GNUC__)
*/

#ifndef MATRIX_HPP
#define MATRIX_HPP 1

#include <cstddef> // std::size_t
#include <utility> // std::swap
#include <stdexcept> // std::length_error, std::out_of_range, std::invalid_argument
#include <type_traits> // std::is_arithmetic
#include <string> // std::string

#if defined(__GNUC__) && !defined(__APPLE__)
#include <bits/functexcept.h> // std::throw_out_of_range_fmt
#include <bits/c++config.h> // __N
#endif // defined(__GNUC__) && !defined(__APPLE__)

#ifdef _MSC_VER
#include <yvals.h> // _STL_REPORT_ERROR, _STL_VERIFY
#endif // _MSC_VER


template<typename T> // Здесь пришлось сделать объявление Matrix, чтобы фунции из detail знали, что это такое
class Matrix;


namespace detail
    /*
     *  Данный namespace содержит служебные функции, которые используются данным классом.
     *  Просьба не использовать функции из этого пространства имён, а тем более, само пространство
     *  имён.
     */
{
    template<typename T>
    void CreateDM(T**& DM, const std::size_t& rows, const std::size_t& cols)
        // Поскольку создавать матрицы я буду часто, то я выделю это в отдельную функцию
    {
        DM = new T*[rows]; // Создание массива массивов
        for (std::size_t i = 0; i < rows; i++) // Создание массивов
        {
            DM[i] = new T[cols];
        }
    }

    template<typename T>
    void EliminateDM(T** DM, const std::size_t& rows)
        // Аналогично CreateDM.
    {
        if (DM != nullptr) // Если DM не нулевой указатель
        {
            for (std::size_t i = 0; i < rows; i++) // Удаление массивов
            {
                delete[] DM[i];
            }
            delete[] DM; // Удаление массива массивов
            DM = nullptr;
        }
    }

    template<typename T>
    void InitializeDM(T** to, T** from, const std::size_t& rows, const std::size_t& cols)
        /* Аналогично CreateDM. Инициализацию от 1-мерного массива и инициализирующего значения
           не стал добавлять т.к. я их использую всего 1 раз. */
    {
        for (std::size_t i = 0; i < rows; i++)
        {
            for (std::size_t j = 0; j < cols; j++)
            {
                to[i][j] = from[i][j];
            }
        }
    }

    template<typename T>
    void RangeCheck(const Matrix<T>& matrix, const std::size_t& row, const std::size_t& col)
        /* Функция RangeCheck. Проверяет, не вышли ли row и col за границы матрицы.
           В случае выхода бросается исключение std::out_of_range */
    {
        if (row >= matrix.Rows() || col >= matrix.Columns())
            // Микро-оптимизация в случае, если выхода за границы нет
        {
            if (row >= matrix.Rows())
            {
#if defined(__GNUC__) && !defined(__APPLE__)
                std::__throw_out_of_range_fmt(__N("In Matrix::At(row, col): "
                                                  "row (which is %zu) >= this->Rows() "
                                                  "(which is %zu)"),
                                              row, matrix.Rows());
#else  // Если используется другой компиляор (не GNU) или используется macOS
                throw std::out_of_range("In Matrix::At(row, col): row >= "
                                        "this->Rows()");
#endif // defined(__GNUC__) && !defined(__APPLE__)
            }
            if (col >= matrix.Columns())
            {
#if defined(__GNUC__) && !defined(__APPLE__)
                std::__throw_out_of_range_fmt(__N("In Matrix::At(row, col): "
                                                  "col (which is %zu) >= this->Columns() "
                                                  "(which is %zu)"),
                                              col, matrix.Columns());
#else  // Если используется другой компиляор (не GNU) или используется macOS
                throw std::out_of_range("In Matrix::At(row, col): col >= "
                                        "this->Columns()");
#endif // defined(__GNUC__) && !defined(__APPLE__)
            }
        }
    }

    template<typename T, typename U>
    void CheckForPossiblityOfDoingAnArithmeticOperation(const Matrix<T>& x, const Matrix<U>& y,
                                                        const std::string& whatOperator)
        /* Функция CheckForPossiblityOfDoingAnArithmeticOperation. Проыеряет, возможно ли
           сделать арифметическую операцию над матрицами. Если невозможно бросается исключение
           std::invalid_argument или вылезет окно с ошибкой (если используется компилятор Microsoft) */
    {
        if (!std::is_arithmetic<T>::value || !std::is_arithmetic<U>::value)
        {
#ifdef _MSC_VER
            _STL_REPORT_ERROR("Wrong template arguments of matrices (to use operator" + whatOperator +
                              " template arguments must be arithmetic).");
#else // Если используется не компилятор Microsoft
            throw std::invalid_argument("In operator" + whatOperator + ": template argument "
                                        "of matrix is not arithmetic. "
                                        "(Both template arguments must be arithmetic).");
#endif // _MSC_VER
        }
        if (x.Rows() != y.Rows() || x.Columns() != y.Columns())
        {
#ifdef _MSC_VER
            _STL_REPORT_ERROR("Matrices have different size (to use operator" + whatOperator +
                              " matrices must have equal size).");
#else // Если используется не компилятор Microsoft
            throw std::invalid_argument("In operator" + whatOperator + ": Matrices have different size."
                                        " (Matrices must have equal size).");
#endif // _MSC_VER
        }
    }
} // namespace detail

template<typename T>
class Matrix
{
public:
    // typedef'ы
    typedef T               ValueType;
    typedef T*              Pointer;
    typedef const T*        ConstPointer;
    typedef T**             DoublePointer;
    typedef const T**       ConstDoublePointer;
    typedef T&              Reference;
    typedef const T&        ConstReference;
    typedef std::size_t     SizeType;
private:
    SizeType PMem_rows; // Кол-во строк в матрице
    SizeType PMem_columns; // Кол-во столбцов в матрице
    DoublePointer PMem_data; // Сама матрица


    class PMem_Proxy
        // Вспомогательный класс. Нужен для реализации двойного индексирования
    {
    private:
        Matrix<ValueType>& PMem_matrix; // Матрица где будет искаться элемент
        SizeType PMem_row; // Строка в которой будет искаться элемент
    public:
        PMem_Proxy(Matrix<ValueType>& matrix, const SizeType& row)
            // Конструктор Proxy
            : PMem_matrix(matrix), PMem_row(row)
        {}

        Reference operator[](const SizeType& col)
          // Оператор индексирования Proxy
        {
#ifdef _MSC_VER
            bool rowSubscriptNotOutOfRange = (this->PMem_row < this->PMem_matrix.PMem_rows
                                                && this->PMem_row >= 0);
            bool columnSubscriptNotOutOfRange = (col < this->PMem_matrix.PMem_columns
                                                   && col >= 0);
            _STL_VERIFY(rowSubscriptNotOutOfRange && columnSubscriptNotOutOfRange,
                       "Matrix subscript out of range.");
            /* Здесь у меня была проблема (не компилилось). Но на
               след. день все прошло. Списал на баг Visual Studio */
#endif // _MSC_VER
            return this->PMem_matrix.PMem_data[this->PMem_row][col];
        }
    };
    
    class PMem_ConstProxy
        /* Константная версия PMem_Proxy (знаю, костыльно, повторное использование кода, ко-ко-ко,
           но другого решения я не придумал, а в гугле по этому поводу ничего нет.
           Для объяснения смотреть PMem_Proxy */
    {
    private:
        const Matrix<ValueType>& PMem_constMatrix;
        SizeType PMem_row;
    public:
        PMem_ConstProxy(const Matrix<ValueType>& constMatrix, const SizeType& row)
            : PMem_constMatrix(constMatrix), PMem_row(row)
        {}

        ConstReference operator[](const SizeType& col) const
        {
#ifdef _MSC_VER
            bool rowSubscriptNotOutOfRange = (this->PMem_row < this->PMem_constMatrix.PMem_rows
                                                && this->PMem_row >= 0);
            bool columnSubscriptNotOutOfRange = (col < this->PMem_constMatrix.PMem_columns
                                                    && col >= 0);
            _STL_VERIFY(rowSubscriptNotOutOfRange && columnSubscriptNotOutOfRange,
                        "Matrix subscript out of range.");
#endif // _MSC_VER
            return this->PMem_constMatrix.PMem_data[this->PMem_row][col];
        }
    };
public:
    Matrix(const SizeType& rows = 0, const SizeType& cols = 0, ConstReference initValue = 0)
        // Стандартный конструктор
        : PMem_rows(rows), PMem_columns(cols)
    {
        if (rows == 0 ^ cols == 0)
            /* Здесь я решил, что нужно бросить исключение т.к. 
               нулевой размер каких-л. "свойств" матрицы может вызвать
               неопределенное поведение */
        {
#ifdef _MSC_VER
            _STL_REPORT_ERROR("Bad Matrix size. In Matrix mustn't be 0 rows xor 0 columns");
#else // Если используется другой компилятор (не Microsoft)
            throw std::length_error("In Matrix constructor: In Matrix mustn't be 0 rows xor 0 columns\n"
                                    "rows == 0 ^ cols == 0 must be false.");
#endif // _MSC_VER
        }
        detail::CreateDM(this->PMem_data, rows, cols);
        for (SizeType i = 0; i < rows; i++) // Инициализация data
        {
            for (SizeType j = 0; j < cols; j++)
            {
                this->PMem_data[i][j] = initValue;
            }
        }
    }

    Matrix(DoublePointer DM, const SizeType& rows, const SizeType& cols,
           bool mustEliminateDM = false)
            /* Конструктор от другой динамической матрицы. Если нужно уничтоить матрицу,
               которая была передана в качестве параметра, посленим аргументом
               нужно указать true */
        : PMem_rows(rows), PMem_columns(cols)
    {
        detail::CreateDM(this->PMem_data, rows, cols);
        detail::InitializeDM(this->PMem_data, DM, rows, cols);
        if (mustEliminateDM)
            // Если флаг mustEliminateDM установлен как true
        {
            detail::EliminateDM(DM, rows);
        }
    }

    Matrix(Pointer SM, const SizeType& rows, const SizeType& cols)
        /* Конструктор от другой статической матрицы. Для использования приведите матрицу к указателю
           на тип матрицы или ссылку на 1-ый элемент (пример: &<название>[0][0] или (<тип>*)<название>) */
        : PMem_rows(rows), PMem_columns(cols)
    {
        detail::CreateDM(this->PMem_data, rows, cols);
        for (SizeType i = 0; i < rows; i++) // Инициализация data
        {
            for (SizeType j = 0; j < cols; j++)
            {
                this->PMem_data[i][j] = SM[i * cols + j];
                /* (i * cols + j) - данная формула позволяет по i и j узнать элемент в матрице,
                    представленной 1-мерным массивом */
            }
        }
    }

    Matrix(const Matrix<ValueType>& other)
        // Конструктор копирования
        : PMem_rows(other.PMem_rows), PMem_columns(other.PMem_columns)
    {
        detail::CreateDM(this->PMem_data, this->PMem_rows, this->PMem_columns);
        detail::InitializeDM(this->PMem_data, other.PMem_data, this->PMem_rows, this->PMem_columns);
    }

    Matrix& operator=(const Matrix<ValueType>& whatAssign)
        // Оператор присваивания. Реализован при помощи Swap
    {
        if (this != &whatAssign)
        {
            Matrix<ValueType>(whatAssign).Swap(*this);
        }
        return *this;
    }


    Matrix(Matrix<ValueType>&& other) noexcept
      // Перемещающий конструктор. Реализован при помощи Swap
      // Обнуление this
      : PMem_rows(0), PMem_columns(0), PMem_data(nullptr)
    {
        this->Swap(other); // Замена обнуленного this и other
    }


    Matrix& operator=(Matrix<ValueType>&& whatMove) noexcept
        // Перемещающий оператор присваивания. Реализован при помощи Swap
    {
        // Обнуление this
        this->PMem_rows = 0;
        this->PMem_columns = 0;
        this->PMem_data = nullptr;
        this->Swap(whatMove); // Замена обнуленного this и whatMove
        return *this;
    }


    ~Matrix() noexcept
        // Деструктор
    {
        detail::EliminateDM(this->PMem_data, this->PMem_rows);
    }



    PMem_Proxy operator[](const SizeType& row)
        // Оператор индексирования. Работает с помощью Proxy
    {
        return PMem_Proxy(*this, row);
    }
    
    PMem_ConstProxy operator[](const SizeType& row) const
        // Константный оператор индексирования
    {
        return PMem_ConstProxy(*this, row);
    }

    Reference At(const SizeType& row, const SizeType& col)
        // Метод At. Безопасная, но медленная замена оператору индексирования
    {
        detail::RangeCheck(*this, row, col);
        return this->PMem_data[row][col];
    }

    ConstReference At(const SizeType& row, const SizeType& col) const
        // Константный At.
    {
        detail::RangeCheck(*this, row, col);
        return this->PMem_data[row][col];
    }



    #define LHS_PTR this // this - это lhs, поэтому, чтобы было удобнее, я сделал этот дефайн.
    void Swap(Matrix<ValueType>& rhs)
        // Метод Swap. Меняет this (LHS_PTR) и rhs местами.
    {
        std::swap(LHS_PTR->PMem_data, rhs.PMem_data);
        std::swap(LHS_PTR->PMem_rows, rhs.PMem_rows);
        std::swap(LHS_PTR->PMem_columns, rhs.PMem_columns);
    }
    #undef LHS_PTR

    void Resize(const SizeType& rows, const SizeType& cols)
        /* Метод Resize. Меняет размер матрицы
           (на самом деле он создает новую rows на cols и инициализирует её) */
    {
        DoublePointer resized;
        detail::CreateDM(resized, rows, cols);
        for (SizeType i = 0; i < rows; i++)
        {
            for (SizeType j = 0; j < cols; j++)
            {
                resized[i][j] = (i < this->PMem_rows && j < this->PMem_columns) ?
                this->PMem_data[i][j] : static_cast<ValueType>(0);
            }
        }
        detail::EliminateDM(this->PMem_data, this->PMem_rows);
        detail::CreateDM(this->PMem_data, rows, cols);
        detail::InitializeDM(this->PMem_data, resized, rows, cols);
        this->PMem_rows = rows;
        this->PMem_columns = cols;
    }

    void Clear() noexcept
        // Метод Clear. Очищает матрицу
    {
        detail::EliminateDM(this->PMem_data, this->PMem_rows);
        detail::CreateDM(this->PMem_data, 0, 0);
        this->PMem_rows = 0;
        this->PMem_columns = 0;
    }

    bool Empty() const noexcept
        // Метод Epmty. Проверяет, пуста ли матрица
    {
        return (this->PMem_rows == 0) && (this->PMem_columns == 0);
    }

    void Transpose() noexcept
        // Метод Transpose. Транспонирет матрицу относительно главной диагонали
    {
        ValueType** transposed;
        detail::CreateDM(transposed, this->PMem_columns, this->PMem_rows);
        for (SizeType i = 0; i < this->PMem_columns; i++)
        {
            for (SizeType j = 0; j < this->PMem_rows; j++)
            {
                transposed[i][j] = this->PMem_data[j][i];
            }
        }
        detail::EliminateDM(this->PMem_data, this->PMem_rows);
        detail::CreateDM(this->PMem_data, this->PMem_columns, this->PMem_rows);
        detail::InitializeDM(this->PMem_data, transposed, this->PMem_columns, this->PMem_rows);
        std::swap(this->PMem_rows, this->PMem_columns);
    }


    SizeType Rows() const noexcept
        // Метод Rows. Вовращает кол-во строк в матрице
    {
        return this->PMem_rows;
    }

    SizeType Columns() const noexcept
        // Метод Columns. Вовращает кол-во столбцов в матрице
    {
        return this->PMem_columns;
    }

    DoublePointer Data() noexcept
        // Метод Data (1 перегрузка). Возвращает адрес массива массивов
    {
        return this->PMem_data;
    }

    Pointer Data(SizeType idx) noexcept
        // Метод Data (2 перегрузка). Возвращает адрес массива под индексом idx в массиве массивов
    {
        return this->PMem_data[idx];
    }

    ConstDoublePointer Data() const noexcept
        // Константный Data 1
    {
        return this->PMem_data;
    }

    ConstPointer Data(SizeType idx) const noexcept
        // Константный Data 2
    {
        return this->PMem_data[idx];
    }
};

template<typename T>
bool operator==(const Matrix<T>& lhs, const Matrix<T>& rhs)
    // Оператор ==. Проверяет 2 матрицы на равенство
{
    if (lhs.Rows() != rhs.Rows() || lhs.Columns() != rhs.Columns())
        // Если размеры у lhs и rhs разные, то смысла в дальнейшей проверке нет.
    {
        return false;
    }
    for (std::size_t i = 0; i < lhs.Rows(); i++) // lhs или rhs, не принципиально т.к они равны
    {
        for (std::size_t j = 0; j < lhs.Columns(); j++)
        {
            if (lhs[i][j] != rhs[i][j])
            {
                return false;
            }
        }
    }
    return true;
}

template<typename T>
inline bool operator!=(const Matrix<T>& lhs, const Matrix<T>& rhs)
    // Оператор !=. Проверяет 2 матрицы на неравенство.
{
    return !(lhs == rhs);
}


template<typename T, typename U>
auto operator+(const Matrix<T>& lhs, const Matrix<U>& rhs) -> Matrix<decltype(lhs[0][0] + rhs[0][0])>
    // Оператор +. Сладывает 2 матрицы. Работает только если Т - арифметический тип
{
    detail::CheckForPossiblityOfDoingAnArithmeticOperation(lhs, rhs, std::string("+"));
    Matrix<decltype(lhs[0][0] + rhs[0][0])> total(lhs.Rows(), lhs.Columns()); // см. комментарий на строке 429.
    for (std::size_t i = 0; i < lhs.Rows(); i++)
    {
        for (std::size_t j = 0; j < lhs.Columns(); j++)
        {
            total[i][j] = lhs[i][j] + rhs[i][j];
        }
    }
    return total;
}


template<typename T>
inline void Swap(Matrix<T>& x, Matrix<T>& y)
    // См. Matrix::Swap()
{
    x.Swap(y);
}

#endif /* MATRIX_HPP */