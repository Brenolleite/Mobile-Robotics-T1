#!/usr/bin/env Rscript
createGraph <- function(){
   data <- read.table("build/points.txt")
   plot(data, xlim=c(-9, 6), ylim=c(-6,8), xlab='X', ylab='Y', type='p', pch=46)
   path <- read.table("build/gt.txt")
   lines(x=path[6:nrow(path),1], y=path[6:nrow(path),2], col="red")
   points(x=path[6,1],y=path[6,2], col="red",lwd=9, pch=23)
}

createGraph()