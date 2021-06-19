FROM node:8.9.4
ENV NODE_ENV=production
RUN apt-get install gcc
WORKDIR /app
COPY ["package.json", "package-lock.json*", "./"]
RUN npm install --production
COPY . .
RUN cd parser && make clean && make
CMD [ "node", "app.js" ]