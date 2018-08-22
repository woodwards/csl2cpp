# csl2cpp

### Convert CSL/ACSL/ACSLX code to C++/cpp/Rcpp using R scripts in RStudio

Since AEgis Technologies' ACSLX product is now defunct, legacy CSL models will need to be translated. 
I chose C++ as the target language due to its wide use, speed, numerical integration capabilities via boost/odeint, and easy integration with R via Rcpp.
The intention is to translate our large CSL model into C++ using R scripting. The resulting model will be able to be run either via Rcpp or as standalone C++.
I thought I would make the scripts publicly available in case others have similar needs.

https://aegistg.com/

http://www.rcpp.org/

https://www.boost.org/doc/libs/1_66_0/libs/numeric/odeint/doc/html/index.html

* see project_guide.txt
* see outstanding_issues.txt
