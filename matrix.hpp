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
  3. #ifdef __GNUC__ - Если используется компилятор GNU
  4. #ifdef _MSC_VER - Если используется компилятор Microsoft
*/

#ifndef MATRIX_HPP
#define MATRIX_HPP 1

#include <cstddef> // std::size_t
#include <utility> // std::move
#include <stdexcept> // std::invalid_argument

#ifdef __GNUC__ // Подключаем в случае использования компилятора GNU
#include <bits/functexcept.h> // std::throw_out_of_range_fmt
#include <bits/c++config.h> // __N
#endif // __GNUC__

#ifdef _MSC_VER
#include <yvals.h> // _STL_REPORT_ERROR
#endif // _MSC_VER

template<typename T>
class Matrix
{
private:
    std::size_t PMem_rows; // Кол-во строк в матрице
    std::size_t PMem_columns; // Кол-во столбцов в матрице
    T** PMem_data; // Сама матрица


    inline T** PMem_CreateDM(const std::size_t& rows, const std::size_t& cols)
        /* Поскольку создавать матрицы я буду часто, то я выделю это в отдельную функцию

        P.S. Данная функция private, потому что я хочу скрыть её от посторонних глаз.
        Можно было использовать static, но static не работает :( (или работает) */
    {
        T** DM = new T*[rows]; // Создание массива массивов
        for (std::size_t i = 0; i < rows; i++) // Создание массивов
        {
            DM[i] = new T[cols];
        }
        return DM;
    }

    inline void PMem_EliminateDM(T** DM, const std::size_t& rows)
        // Аналогично CreateDM.
    {
        for (std::size_t i = 0; i < rows; i++) // Удаление массивов
        {
            delete[] DM[i];
        }
        delete[] DM; // Удаление массива массивов
    }

    inline void PMem_InitializeDM(T** to, T** from, const std::size_t& rows, const std::size_t& cols)
        /* Аналогично CreateMatrix. Инициализацию от 1-мерного массива и инициализирующего значения
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
             _STL_VERIFY(this->PMem_row >= this->Pmem_matrix.PMem_rows || col >= this->Pmem_matrix.PMem_columns,
                        "Matrix subscript out of range.");
            #endif
            return this->PMem_matrix.PMem_data[this->PMem_row][col];
        }
    };


    void PMem_RangeCheck(std::size_t i, std::size_t j)
        /* Метод RangeCheck. Проверяет, не вышли ли i и j за границы матрицы. В случае выхода бросается исключение
          std::out_of_range */
    {
        if (i >= this->PMem_rows || j >= this->PMem_columns)
        {
            if (i >= this->PMem_rows)
            {
                #ifdef __GNUC__
                std::__throw_out_of_range_fmt(__N("In Matrix::PMem_RangeCheck(): i (which is %zu) >= "
                                                  "this->GetRows() "
                                                  "(which is %zu)"),
                                              i, this->PMem_rows);
                #elif defined(_MSC_VER)
                _STL_REPORT_ERROR("Matrix subscript out of range "
                                  "(first argument >= this->GetRows())");
                #else
                throw std::out_of_range("In Matrix::PMem_RangeCheck(): first argument >= "
                                        "this->GetRows()");
                #endif // __GNUC__ и _MSC_VER
            }
            if (j >= this->PMem_columns)
            {
                #ifdef __GNUC__
                std::__throw_out_of_range_fmt(__N("In Matrix::PMem_RangeCheck(): j (which is %zu) >= "
                                                  "this->GetColumns() "
                                                  "(which is %zu)"),
                                              j, this->PMem_columns);
                #elif defined(_MSC_VER)
                _STL_REPORT_ERROR("Matrix subscript out of range "
                                  "(second argument >= this->GetColumns())");
                #else
                throw std::out_of_range("In Matrix::PMem_RangeCheck(): second argument >= "
                                        "this->GetColumns()");
                #endif // __GNUC__ и _MSC_VER
            }
        }
    }
public:
    Matrix(const std::size_t& rows = 0, const std::size_t& cols = 0, const T& initValue = 0)
        // Стандартный конструктор
        : PMem_rows(rows), PMem_columns(cols), PMem_data(PMem_CreateDM(rows, cols))
    {
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
        : PMem_rows(other.rows), PMem_columns(other.columns),
          PMem_data(PMem_CreateDM(this->PMem_rows, this->columns))
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

    T& At(const std::size_t& row, const std::size_t& col)
        // Метод At. Безопасная, но медленная замена оператору индексирования
    {
        this->PMem_RangeCheck(row, col);
        return this->PMem_data[row][col];
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


    std::size_t GetRows() const
        // Метод GetRows. Вовращает кол-во строк в матрице
    {
        return this->PMem_rows;
    }

    std::size_t GetColumns() const
        // Метод GetColumns. Вовращает кол-во столбцов в матрице
    {
        return this->PMem_columns;
    }

    T** GetDataAddress() noexcept
        // Метод GetDataAddress (1 перегрузка). Возвращает адрес массива массивов
    {
        return this->PMem_data;
    }

    T* GetDataAddress(std::size_t idx) noexcept
        // Метод GetDataAddress (2 перегрузка). Возвращает адрес массива под индексом idx в массиве массивов
    {
        return this->PMem_data[idx];
    }
};

#endif /* MATRIX_HPP */