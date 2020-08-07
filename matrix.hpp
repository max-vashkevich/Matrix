//
//  matrix.hpp
//  Matrix
//
//  Created by Maxim Vashkevich on 7/31/20.
//  Copyright © 2020 Maxim Vashkevich. All rights reserved.
//

/* Вся реализация размещена в hpp файле т.к. данный класс — шаблонный */

/*
    Условные обозначения:

    1. DM - динамическая матрица (не обертка, просто 2 указателя на ValueType)
    2. SM - статическая матрица
*/

#ifndef MATRIX_HPP
#define MATRIX_HPP 1

#include <cstddef> // std::size_t
#include <ostream> // std::ostream

template<typename T>
class Matrix
{
public:
    using ValueType = T;
    using SizeType = std::size_t;
private:
    SizeType rows;
    SizeType columns;
    ValueType** data;

    
    inline ValueType** CreateDM(const SizeType& rows, const SizeType& cols)
        /* Поскольку создавать матрицы я буду часто, то я выделю это в отдельную функцию
           
         P.S. Данная функция private, потому что я хочу скрыть её от посторонних глаз.
         Можно было использовать static, но static не работает (возможно он нормально работает, просто я ничего не
         понимаю) */
    {
        ValueType** DM = new ValueType*[rows];
        for (SizeType i = 0; i < rows; i++)
        {
            DM[i] = new ValueType[cols];
        }
        return DM;
    }

    inline void EliminateDM(ValueType** DM, const SizeType& rows)
        // Аналогично CreateDM.
    {
        for (SizeType i = 0; i < rows; i++)
        {
            delete[] DM[i];
        }
        delete[] DM;
    }
    
    inline void InitializeDM(ValueType** to, ValueType** from,
                             const SizeType& rows, const SizeType& cols)
        /* Аналогично CreateMatrix. Инициализацию от 1-мерного массива и инициализирующего значения не стал добавлять т.к. я их использую всего 1 раз. */
        
    {
        for (SizeType i = 0; i < rows; i++)
        {
            for (SizeType j = 0; j < cols; j++)
            {
                to[i][j] = from[i][j];
            }
        }
    }


    class Proxy
        // Вспомогательный класс. Нужен для реализации двойного индексирования
    {
    private:
        Matrix<ValueType>& matrix;
        SizeType row;
    public:
        Proxy(Matrix<ValueType>& matrix, const SizeType& row)
            // Конструктор Proxy
            : matrix(matrix), row(row)
        {}

        ValueType& operator[](const SizeType& col)
            // Оператор индексирования Proxy
        {
            return this->matrix.data[this->row][col];
        }
    };
public:
    Matrix(const SizeType& rows = 0, const SizeType& cols = 0, const ValueType& initValue = 0)
        // Стандартный конструктор
        : rows(rows), columns(cols), data(CreateDM(rows, cols))
    {
        for (SizeType i = 0; i < rows; i++) // Инициализация data
        {
            for (SizeType j = 0; j < cols; j++)
            {
                this->data[i][j] = initValue;
            }
        }
    }
    
    Matrix(ValueType** DM, const SizeType& rows, const SizeType& cols,
           const bool& mustEliminateDM = false)
        /* Конструктор от другой динамической матрицы. Если нужно уничтоить матрицу, которая была передана в
           качестве параметра, посленим аргументом нужно указать true */
        : rows(rows), columns(cols), data(CreateDM(rows, cols))
    {
        InitializeDM(this->data, DM, rows, cols);
        if (mustEliminateDM)
            // Если флаг mustEliminateDM установлен как true
        {
            EliminateDM(DM, rows);
        }
    }
    
    Matrix(ValueType* SM, const SizeType& rows, const SizeType& cols)
        /* Конструктор от другой статической матрицы. Для использования приведите матрицу к указателю на тип матрицы
           или ссылку на 1-ый элемент (пример: &<название>[0][0] или (<тип>*)<название>) */
        : rows(rows), columns(cols), data(CreateDM(rows, cols))
    {
        for (SizeType i = 0; i < rows; i++) // Инициализация data
        {
            for (SizeType j = 0; j < cols; j++)
            {
                this->data[i][j] = SM[i * cols + j];
                /* (i * cols + j) - данная формула позволяет по i и j узнать элемент в матрице,
                   представленной 1-мерным массивом */
            }
        }
    }
    
    
    Matrix(const Matrix<ValueType>& other)
        // Конструктор копирования
        : rows(other.rows), columns(other.columns), data(CreateDM(this->rows, this->columns))
    {
        InitializeDM(this->data, other.data, this->rows, this->columns);
    }
    
    Matrix& operator=(const Matrix<ValueType>& whatAssign)
        // Оператор присваивания
    {
        if (this != &whatAssign)
        {
            Matrix<ValueType>(whatAssign).Swap(*this);
        }
        return *this;
    }
    
    
    Matrix(Matrix<ValueType>&& other)
        // Перемещающий конструктор
        : rows(0), columns(0), data(nullptr)
    {
        this->Swap(other);
    }
    
    Matrix& operator=(Matrix<ValueType>&& whatMove)
        // Перемещающий оператор присваивания
    {
        this->rows = 0;
        this->columns = 0;
        this->data = nullptr;
        this->Swap(whatMove);
        return *this;
    }
    
    
    ~Matrix()
        // Деструктор
    {
        EliminateDM(this->data, this->rows);
    }
    

    
    Proxy operator[](const SizeType& row)
        // Оператор индексирования
    {
        return Proxy(*this, row);
    }

    ValueType& At(SizeType row, SizeType col)
    {
        return this->data[row][col];
    }




    
    #define LHS_PTR this // this - это lhs, поэтому, чтобы было удобнее, я сделал этот дефайн.
    void Swap(Matrix<ValueType>& rhs)
        // Метод Swap. Меняет this (LHS_PTR) и rhs местами.
    {
        // При замене data можно использовать присваивание "напрямую" т.к. используется std::move
        ValueType** tmpValueTypeDM = std::move(LHS_PTR->data);
        LHS_PTR->data = std::move(rhs.data);
        rhs.data = std::move(tmpValueTypeDM);

        SizeType tmpSizeType = LHS_PTR->rows;
        LHS_PTR->rows = rhs.rows;
        rhs.rows = tmpSizeType;

        tmpSizeType = LHS_PTR->columns;
        LHS_PTR->columns = rhs.columns;
        rhs.columns = tmpSizeType;
    }
    #undef LHS_PTR

    void Resize(const SizeType& rows, const SizeType& cols)
        // Метод Resize. Меняет размер матрицы
    {
        ValueType** resized = CreateDM(rows, cols);
        for (SizeType i = 0; i < rows; i++)
        {
            for (SizeType j = 0; j < cols; j++)
            {
                resized[i][j] = (i < this->rows && j < this->columns) ?
                                this->data[i][j] : static_cast<ValueType>(0);
            }
        }
        EliminateDM(this->data, this->rows);
        this->data = CreateDM(rows, cols);
        InitializeDM(this->data, resized, rows, cols);
        this->rows = rows;
        this->columns = cols;
    }


    SizeType GetRows() const
        // Метод GetRows. Вовращает кол-во строк в матрице
    {
        return this->rows;
    }

    SizeType GetColumns() const
        // Метод GetColumns. Вовращает кол-во столбцов в матрице
    {
        return this->columns;
    }

    ValueType** GetDataAddress()
        // Метод GetDataAddress. Возвращает адрес data.
    {
        return this->data;
    }
};

#endif /* MATRIX_HPP */
