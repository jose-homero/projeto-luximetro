# Desenvolvimento de Luxímetro Espectral

![Status](https://img.shields.io/badge/Status-Concluído-success)
![Linguagem](https://img.shields.io/badge/Linguagens-C%2B%2B%20%7C%20Python%20%7C%20JavaScript-blue)
![Hardware](https://img.shields.io/badge/Hardware-ESP32--C3%20%7C%20AS7341-orange)
![Protocolo](https://img.shields.io/badge/Protocolo-MQTT-yellow)
![Licença](https://img.shields.io/badge/Licença-MIT-lightgrey)

Este repositório contém os códigos-fonte, algoritmos de processamento e a interface web desenvolvidos para o **Projeto de Final de Curso em Engenharia Física (UFRGS)**. O projeto consiste em um dispositivo portátil capaz de medir irradiância espectral (Visível + NIR).

## 📋 Visão Geral

O sistema foi projetado para superar as limitações dos luxímetros comerciais convencionais, oferecendo uma resolução espectral de **10 canais** (400nm a 956nm).

### Funcionalidades Principais
* **Hardware IoT:** Coleta de dados espectrais com sensor AS7341 e transmissão via MQTT (ESP32-C3).
* **Dashboard Web:** Visualização das contagens por canal.
* **Análise de Dados:** Algoritmos em Python (**Elastic Net**, **Ridge Regression**) para reconstruir curvas espectrais contínuas a partir de dados discretos.

---

## 🛠️ Arquitetura do Projeto

O repositório está organizado da seguinte forma:

```text
luximetro-espectral/
│
├── 📂 esp32/              # Firmware C++ para o microcontrolador
│   ├── esp32.ino          # Código principal (Leitura sensor + MQTT)
│   └── DEPENDENCIES.txt   # Lista de bibliotecas necessárias
│
├── 📂 dashboard/          # Interface Web (Front-end)
│   └── dashboard.html     # Dashboard visual (HTML5 + JS)
│
├── 📂 análise-dados/      # Scripts de Processamento (Python)
│   ├── datasets           # Dados brutos para teste (CSVs)
│   ├── tcc_minimo.py      # Script principal de calibração e gráficos
│   └── requirements.txt   # Bibliotecas Python necessárias
│
└── README.md              # Documentação do projeto
