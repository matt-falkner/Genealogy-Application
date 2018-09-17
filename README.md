# Genealogy-Application
Create, Edit, Upload and Download Geneaology Files in the GEDCOM Standards 

## Pre Conditions 
This application requires Node v8.9.4 LTS, I would recommend [downloading nvm](https://github.com/creationix/nvm) which allows you to quickly switch between node and npm distrobutions. 
```bash
node -v //check your version
nvm install 8.9.4 //download and install specific version of node
nvm use 8.9.4 //choose to use that specific version
```

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
npm run dev 
```
The application currently defaults to port 3000. IE. http:localhost:3000

## Current Features
- View relationships between ancestors / descendants and Generations. 
- Generic C GEDCOM parser bundled
- create and download simple gedcom files 
- introspect and store current files 

## Future Features
- Login System allowing users to save their files and changes to accounts 
- Add to the 'insert new individual' feature allowing users to connect relatives through the GUI. 
- Displaying the data in a tree format, rather than list, similar to ancestry.com's formating!


## About the Project 

There are 4 aspects to the project. 

1. Writen in C, the shared library parses, creates, and most importantly analyizes GEDCOM Geneology Files.
2. Writen in C, the API interface that will allow Javascript to interact with the API through JSON strings. 
3. The Javascript, HTML, CSS Front End, Node.JS Backend that communicates with the C backend.
4. Javascript interface with MYSQL database for cahcing users Geneology files meta data. IE. Instead of constantly quering the C parser for data, use already parsed data. [This is subject to change] 
