## find_suid
## Part of the ansvif tool chain
##    __ _  _  __   ___  __  ____ ____
##   /  ( \/ )/ _\ / __)/ _\/ ___(_  _)
##  (  O )  (/    ( (_ /    \___ \ )(
##   \__(_/\_\_/\_/\___\_/\_(____/(__)

if [ $# -eq 0 ]
  then
    echo "ansvif toolchain -- A Not So Very Intelligent Fuzzer";
    echo "Usage:";
    echo " $0 /bin /usr/bin";
  exit 0;
fi;
find $@ -perm /6000 2>/dev/null
