# -*- coding: utf-8 -*-
"""
Created on Tue Jan 13 13:12:39 2026

@author: mateu
"""

import numpy as np
import pandas as pd

basedeDados = pd.read_csv('admission.csv', delimiter=';')
X=basedeDados.iloc[:,:-1].values
Y=basedeDados.iloc[:,-1].values

from sklearn.impute import SimpleImputer
imputer = SimpleImputer(missing_values=np.nan, strategy='median')
imputer = imputer.fit_transform(X[:,1:])

from sklearn.preprocessing import LabelEncoder
LabelEncoder_X= LabelEncoder()
X[:,0] = LabelEncoder_X.fit_transform(X[:,0])
print(X)

X= X[:,1:]
D = pd.get_dummies(X[:,0])
X= np.insert(X, 0, D.values,axis =1)
print(X)

from sklearn.model_selection import train_test_split
XTrain, Xtest, yTrain, yTest = train_test_split(X,Y, test_size = 0.2)
print(XTrain)