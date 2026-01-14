import numpy as np
import pandas as pd 

# Lê o arquivo CSV chamado "svbr.csv" usando ponto e vírgula como delimitador
baseDeDados = pd.read_csv('svbr.csv', delimiter=';')

# Seleciona TODAS as linhas e TODAS as colunas da base e transforma em matriz (array do NumPy)
X = baseDeDados.iloc[:, :].values

# Importa o SimpleImputer, que preenche valores faltantes (NaN)
from sklearn.impute import SimpleImputer

# Cria o imputer usando a estratégia "median" (troca valores faltantes pela mediana da coluna)
imputer = SimpleImputer(missing_values=np.nan, strategy='median')

# Ajusta o imputer apenas nas colunas 1 e 2 da base (ignora a coluna 0)
imputer = imputer.fit(X[:, 1:3])

# Transforma os valores, substituindo os NaN pela mediana calculada
# Converte os valores resultantes para string
X = imputer.transform(X[:, 1:3]).astype(str)

# Insere a primeira coluna original (ID, nome, código, etc.) de volta na posição 0 do array
X = np.insert(X, 0, baseDeDados.iloc[:, 0].values, axis=1)

# Exibe o resultado final no console
print(X)
