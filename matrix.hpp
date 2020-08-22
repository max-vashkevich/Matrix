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
  5. !defined(__APPLE__) - Если используется не MacOS (В Xcode стоит GNU, однако он не знвет о файлах c++config
     и functexcept) (используется в свяске с defined(__GNUC__)
*/

#ifndef MATRIX_HPP
#define MATRIX_HPP 1

#include <cstddef> // std::size_t
#include <utility> // std::move
#include <stdexcept> // std::length_error, std::out_of_range

#if defined(__GNUC__) && !defined(__APPLE__)
#include <bits/functexcept.h> // std::throw_out_of_range_fmt
#include <bits/c++config.h> // __N
#endif // defined(__GNUC__) && !defined(__APPLE__)

#ifdef _MSC_VER
#include <yvals.h> // _STL_REPORT_ERROR, _STL_VERIFY
#endif // _MSC_VER

template<typename T>
class Matrix
{
private:
    std::size_t PMem_rows; // Кол-во строк в матрице
    std::size_t PMem_columns; // Кол-во столбцов в матрице
    T** PMem_data; // Сама матрица


    T** PMem_CreateDM(const std::size_t& rows, const std::size_t& cols)
        /* Поскольку создавать матрицы я буду часто, то я выделю это в отдельную функцию

        P.S. Данная функция private, потому что я хочу скрыть её от посторонних глаз.
        Можно было использовать static, но static не работает :( (или работает) (спрятал функцию костылно короч) */
    {
        T** DM = new T*[rows]; // Создание массива массивов
        for (std::size_t i = 0; i < rows; i++) // Создание массивов
        {
            DM[i] = new T[cols];
        }
        return DM;
    }

    void PMem_EliminateDM(T** DM, const std::size_t& rows)
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

    void PMem_InitializeDM(T** to, T** from, const std::size_t& rows, const std::size_t& cols)
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


    class PMem_Proxy
        // Вспомогательный класс. Нужен для реализации двойного индексирования
    {
    private:
        Matrix<T>& PMem_matrix; // Матрица где будет искаться элемент
        std::size_t PMem_row; // Строка в которой будет искаться элемент
    public:
        PMem_Proxy(Matrix<T>& matrix, const std::size_t& row)
          // Конструктор Proxy
            : PMem_matrix(matrix), PMem_row(row)
        {}

        T& operator[](const std::size_t& col)
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
        const Matrix<T>& PMem_constMatrix;
        std::size_t PMem_row;
    public:
        PMem_ConstProxy(const Matrix<T>& constMatrix, const std::size_t& row)
            : PMem_constMatrix(constMatrix), PMem_row(row)
        {}

        const T& operator[](const std::size_t& col) const
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


    void PMem_RangeCheck(std::size_t i, std::size_t j)
        /* Метод RangeCheck. Проверяет, не вышли ли i и j за границы матрицы. В случае выхода бросается исключение
          std::out_of_range */
    {
        if (i >= this->PMem_rows || j >= this->PMem_columns)
            // Микро-оптимизация в случае, если выхода за границы нет
        {
            if (i >= this->PMem_rows)
            {
#if defined(__GNUC__) && !defined(__APPLE__)
                std::__throw_out_of_range_fmt(__N("In Matrix::PMem_RangeCheck(): i (which is %zu) >= "
                                                  "this->GetRows() "
                                                  "(which is %zu)"),
                                              i, this->PMem_rows);
#else  // Если используется другой компиляор (не GNU) или используется macOS
                throw std::out_of_range("In Matrix::PMem_RangeCheck(): first argument >= "
                                        "this->GetRows()");
#endif // defined(__GNUC__) && !defined(__APPLE__)
            }
            if (j >= this->PMem_columns)
            {
#if defined(__GNUC__) && !defined(__APPLE__)
                std::__throw_out_of_range_fmt(__N("In Matrix::PMem_RangeCheck(): j (which is %zu) >= "
                                                  "this->GetColumns() "
                                                  "(which is %zu)"),
                                              j, this->PMem_columns);
#else // Если используется другой компиляор (не GNU) или используется macOS
                throw std::out_of_range("In Matrix::PMem_RangeCheck(): second argument >= "
                                        "this->GetColumns()");
#endif // defined(__GNUC__) && !defined(__APPLE__)
            }
        }
    }
public:
    Matrix(const std::size_t& rows = 0, const std::size_t& cols = 0, const T& initValue = 0)
        // Стандартный конструктор
        : PMem_rows(rows), PMem_columns(cols), PMem_data(PMem_CreateDM(rows, cols))
    {
        if (rows == 0 ^ cols == 0)
            /* Здесь я решил, что нужно бросить исключение т.к. 
               нулевой размер каких-л. "свойств" матрицы может вызвать
               неопределенное поведение */
        {
#ifdef _MSC_VER
            _STL_REPORT_ERROR("Bad Matrix size.");
#else // Если используется другой компилятор (не Microsoft)
            throw std::length_error("In Matrix mustn't be 0 rows xor 0 columns\n"
                                    "rows == 0 ^ cols == 0 must be false.");
#endif // _MSC_VER
        }
        for (std::size_t i = 0; i < rows; i++) // Инициализация data
        {
            for (std::size_t j = 0; j < cols; j++)
            {
                this->PMem_data[i][j] = initValue;
            }
        }
    }

    Matrix(T** DM, const std::size_t& rows, const std::size_t& cols,
           const bool& mustEliminateDM = false)
            /* Конструктор от другой динамической матрицы. Если нужно уничтоить матрицу, которая была передана в
               качестве параметра, посленим аргументом нужно указать true */
        : PMem_rows(rows), PMem_columns(cols), PMem_data(PMem_CreateDM(rows, cols))
    {
        PMem_InitializeDM(this->PMem_data, DM, rows, cols);
        if (mustEliminateDM)
            // Если флаг mustEliminateDM установлен как true
        {
            PMem_EliminateDM(DM, rows);
        }
    }

    Matrix(T* SM, const std::size_t& rows, const std::size_t& cols)
        /* Конструктор от другой статической матрицы. Для использования приведите матрицу к указателю на тип матрицы
           или ссылку на 1-ый элемент (пример: &<название>[0][0] или (<тип>*)<название>) */
        : PMem_rows(rows), PMem_columns(cols), PMem_data(PMem_CreateDM(rows, cols))
    {
        for (std::size_t i = 0; i < rows; i++) // Инициализация data
        {
            for (std::size_t j = 0; j < cols; j++)
            {
                this->PMem_data[i][j] = SM[i * cols + j];
                /* (i * cols + j) - данная формула позволяет по i и j узнать элемент в матрице,
                    представленной 1-мерным массивом */
            }
        }
    }


    Matrix(const Matrix<T>& other)
        // Конструктор копирования
        : PMem_rows(other.PMem_rows), PMem_columns(other.PMem_columns),
          PMem_data(PMem_CreateDM(this->PMem_rows, this->PMem_columns))
    {
        PMem_InitializeDM(this->PMem_data, other.PMem_data, this->PMem_rows, this->PMem_columns);
    }

    Matrix& operator=(const Matrix<T>& whatAssign)
        // Оператор присваивания. Реализован при помощи Swap
    {
        if (this != &whatAssign)
        {
            Matrix<T>(whatAssign).Swap(*this);
        }
        return *this;
    }


    Matrix(Matrix<T>&& other) noexcept
      // Перемещающий конструктор. Реализован при помощи Swap
      // Обнуление this
      : PMem_rows(0), PMem_columns(0), PMem_data(nullptr)
    {
        this->Swap(other); // Замена обнуленного this и other
    }


    Matrix& operator=(Matrix<T>&& whatMove) noexcept
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
        PMem_EliminateDM(this->PMem_data, this->PMem_rows);
    }



    PMem_Proxy operator[](const std::size_t& row)
        // Оператор индексирования. Работает с помощью Proxy
    {
        return PMem_Proxy(*this, row);
    }
    
    PMem_ConstProxy operator[](const std::size_t& row) const
    {
        return PMem_ConstProxy(*this, row);
    }

    T& At(const std::size_t& row, const std::size_t& col)
        // Метод At. Безопасная, но медленная замена оператору индексирования
    {
        this->PMem_RangeCheck(row, col);
        return this->PMem_data[row][col];
    }
    
    const T& At(const std::size_t& row, const std::size_t& col) const
        // Константный At.
    {
        return this->At(row, col);
    }



    #define LHS_PTR this // this - это lhs, поэтому, чтобы было удобнее, я сделал этот дефайн.
    void Swap(Matrix<T>& rhs)
        // Метод Swap. Меняет this (LHS_PTR) и rhs местами.
    {
        // При замене data можно использовать присваивание "напрямую" т.к. используется std::move
        T** tmpTDM = std::move(LHS_PTR->PMem_data);
        LHS_PTR->PMem_data = std::move(rhs.PMem_data);
        rhs.PMem_data = std::move(tmpTDM);

        // Замена rows и columns самая обыконвенная 
        std::size_t tmp__size_t_ = LHS_PTR->PMem_rows;
        LHS_PTR->PMem_rows = rhs.PMem_rows;
        rhs.PMem_rows = tmp__size_t_;

        tmp__size_t_ = LHS_PTR->PMem_columns;
        LHS_PTR->PMem_columns = rhs.PMem_columns;
        rhs.PMem_columns = tmp__size_t_;
    }
    #undef LHS_PTR

    void Resize(const std::size_t& rows, const std::size_t& cols)
        // Метод Resize. Меняет размер матрицы (на самом деле он создает новую rows на cols и инициализирует её)
    {
        T** resized = PMem_CreateDM(rows, cols);
        for (std::size_t i = 0; i < rows; i++)
        {
            for (std::size_t j = 0; j < cols; j++)
            {
                resized[i][j] = (i < this->PMem_rows && j < this->PMem_columns) ?
                this->PMem_data[i][j] : static_cast<T>(0);
            }
        }
        PMem_EliminateDM(this->PMem_data, this->PMem_rows);
        this->PMem_data = PMem_CreateDM(rows, cols);
        PMem_InitializeDM(this->PMem_data, resized, rows, cols);
        this->PMem_rows = rows;
        this->PMem_columns = cols;
    }

    void Clear() noexcept
        // Метод Clear. Очищает матрицу
    {
        PMem_EliminateDM(this->PMem_data, this->PMem_rows);
        this->PMem_data = PMem_CreateDM(0, 0);
        this->PMem_rows = 0;
        this->PMem_columns = 0;
    }

    bool Empty() const noexcept
        // Метод Epmty. Проверяет, пуста ли матрица
    {
        return this->PMem_rows == 0 && this->PMem_columns == 0;
    }

    std::size_t Rows() const
        // Метод Rows. Вовращает кол-во строк в матрице
    {
        return this->PMem_rows;
    }

    std::size_t Columns() const
        // Метод Columns. Вовращает кол-во столбцов в матрице
    {
        return this->PMem_columns;
    }

    T** Data() noexcept
        // Метод Data (1 перегрузка). Возвращает адрес массива массивов
    {
        return this->PMem_data;
    }

    T* Data(std::size_t idx) noexcept
        // Метод Data (2 перегрузка). Возвращает адрес массива под индексом idx в массиве массивов
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
    // Оператор != =. Проверяет 2 матрицы на неравенство.
{
    return !(lhs == rhs);
}

#endif /* MATRIX_HPP */
