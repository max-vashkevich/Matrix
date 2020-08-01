//
//  matrix.hpp
//  Matrix
//
//  Created by Maxim Vashkevich on 7/31/20.
//  Copyright © 2020 Maxim Vashkevich. All rights reserved.
//

/* Вся реализация размещена в hpp файле т.к. данный класс — шаблонный */

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cstddef> // std::size_t
#include <utility> // std::swap

template<typename T>
class Matrix
{
public: using ValueType = T;
private:
    std::size_t rows;
    std::size_t columns;
    ValueType** data;
    
    
    inline ValueType** CreateMatrix(std::size_t rows, std::size_t cols)
        /* Поскольку создавать матрицы я буду часто, то я выделю это в отдельную функцию
           
         P.S. Данная функция private, потому что я хочу скрыть её от посторонних глаз.
         Можно было использовать static, но static не работает (возможно он нормально работает, просто я ничего не
         понимаю) */
    {
        ValueType** matrix = new ValueType*[rows];
        for (std::size_t i = 0; i < rows; i++)
        {
            matrix[i] = new ValueType[cols];
        }
        return matrix;
    }
    
    inline void InitializeMatrix(ValueType** to, ValueType** from, std::size_t rows, std::size_t cols)
        /* Аналогично CreateMatrix. Инициализацию от 1-мерного массива и инициализирующего значения не стал добавлять
           т.к. я их использую всего 1 раз. */
        
    {
        for (std::size_t i = 0; i < rows; i++)
        {
            for (std::size_t j = 0; j < cols; j++)
            {
                to[i][j] = from[i][j];
            }
        }
    }
public:
    Matrix(std::size_t rows = 0, std::size_t cols = 0, ValueType initValue = 0)
        // Стандартный конструктор
    {
        this->rows = rows;
        this->columns = cols;
        this->data = CreateMatrix(rows, cols);
        for (std::size_t i = 0; i < rows; i++) // Инициализация data
        {
            for (std::size_t j = 0; j < cols; j++)
            {
                this->data[i][j] = initValue;
            }
        }
    }
    
    Matrix(ValueType** dynamicMatrix, std::size_t rows, std::size_t cols,
           bool eliminateAfterCreation = false)
        /* Конструктор от другой динамической матрицы. Если нужно уничтоить матрицу, которая была передана в
           качестве параметра, посленим аргументом нужно указать true */
    {
        this->rows = rows;
        this->columns = cols;
        this->data = CreateMatrix(rows, cols);
        InitializeMatrix(this->data, dynamicMatrix, rows, cols);
        if (eliminateAfterCreation)
            // Если флаг eliminateAfterCreation установлен как true
        {
            InitializeMatrix(this->data, dynamicMatrix, rows, cols);
            for (std::size_t i = 0; i < rows; i++) // удаление matrix
            {
                delete[] dynamicMatrix[i];
            }
            delete[] dynamicMatrix;
        }
    }
    
    Matrix(ValueType* staticMatrix, std::size_t rows, std::size_t cols)
        /* Конструктор от другой статической матрицы. Для использования приведите матрицу к указателю на тип матрицы
           или ссылку на 1-ый элемент (пример: &<название>[0][0] или (<тип>*)<название>) */
    {
        this->rows = rows;
        this->columns = cols;
        this->data = CreateMatrix(rows, cols);
        for (std::size_t i = 0; i < rows; i++) // Инициализация data
        {
            for (std::size_t j = 0; j < cols; j++)
            {
                this->data[i][j] = staticMatrix[i * cols + j];
                /* (i * cols + j) - данная формула позволяет по i и j узнать элемент в матрице,
                   представленной 1-мерным массивом */
            }
        }
    }
    
    
    Matrix(const Matrix<ValueType>& other)
        // Конструктор копирования
    {
        this->rows = other.rows;
        this->columns = other.columns;
        this->data = CreateMatrix(this->rows, this->columns);
        InitializeMatrix(this->data, other.data, this->rows, this->columns);
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
    {
        this->Swap(other);
    }
    
    Matrix& operator=(Matrix<ValueType>&& whatMove)
        // Перемещающий оператор присваивания
    {
        this->Swap(whatMove);
        return *this;
    }
    
    
    ~Matrix()
        // Деструктор
    {
        for (std::size_t i = 0; i < this->rows; i++)
        {
            delete[] this->data[i];
        }
        delete[] this->data;
    }
    
    
    
    void Swap(Matrix<ValueType>& other)
        // Метод Swap. Меняет this и other местами.
    {
        std::swap(this->rows, other.rows);
        std::swap(this->columns, other.columns);
        std::swap(this->data, other.data);
    }
};

#endif /* MATRIX_HPP */
