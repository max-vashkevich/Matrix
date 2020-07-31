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

#include <cstddef>
#include <stdexcept>

template<typename T>
class Matrix
{
private:
    std::size_t rows;
    std::size_t columns;
    T** data;
public:
    using ValueType = T;
    
    // Конструкторы
    Matrix(std::size_t rows = 1, std::size_t columns = 1, ValueType initValue = 0)
        // Стандартный конструктор
    {
        if (rows == 0 ^ columns == 0)
            // Нельзя создать матрицу с 0 строками (исключающее)или 0 столбцами
        {
            throw std::invalid_argument("Cannot create matrix with 0 rows (x)or 0 columns");
        }
        // Создание матрицы (начало)
        this->data = new ValueType*[rows];
        for (std::size_t i = 0; i < rows; i++)
        {
            this->data = new ValueType[columns];
        }
        /* Создание матрицы (конец),
           Инициализация матрицы (начало) */
        for (std::size_t i = 0; i < rows; i++)
        {
            for (std::size_t j = 0; j < columns; j++)
            {
                this->data[i][j] = initValue;
            }
        }
        /* Инициализация матрицы (конец)
           Инициализация других полей класса (начало) */
        this->rows = rows;
        this->columns = columns;
        // Инициализация других полей класса (конец)
    }
    
    Matrix(ValueType** matrix, std::size_t rows, std::size_t columns,
           bool eliminateAfterCreation = false)
        /* Конструктор от другой динамической матрицы. Если нужно уничтоить матрицу, которая была передана в
           качестве параметра, посленим аргументом нужно указать true */
    {
        if (eliminateAfterCreation)
        {
            this->data = matrix;
            for (std::size_t i = 0; i < rows; i++)
            {
                delete[] matrix[i];
            }
            delete[] matrix;
            return;
        }
        for (std::size_t i = 0; i < rows; i++)
        {
            for (std::size_t j = 0; j < columns; j++)
            {
                this->data[i][j] = matrix[i][j];
            }
        }
    }
};

#endif /* MATRIX_HPP */
