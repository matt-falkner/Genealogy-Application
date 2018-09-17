# Genealogy-Application
Create, Edit, Upload and Download Geneology Files in the GEDCOM Standards 

Features
- View relationships between ancestors / descendants 

## Pre Conditions 
```bash
node -v 
```
This project is dependant on nodejs v8.9.4 (LTS)


Started as the Angel of Death semester project, I've refined from it's academic teaching purposes into an actually useful large scale program. 

## How to Download

```bash 
git clone https://github.com/BaronLR/Genealogy-Application.git
```

## How to Build

```bash 
cd Genealogy-Application 
cd parser 
make
cd ..
npm install 
```

## How to Run
```bash
npm run dev 8080
```

## About the Project 

There are 4 aspects to the project. 

1. Writen in C, the shared library parses, creates, and most importantly analyizes GEDCOM Geneology Files.
2. Writen in C, the API interface that will allow Javascript to interact with the API through JSON strings. 
3. The Javascript, HTML, CSS Front End, Node.JS Backend that communicates with the C backend.
4. Javascript interface with MYSQL database for cahcing users Geneology files meta data. IE. Instead of constantly quering the C parser for data, use already parsed data. 





