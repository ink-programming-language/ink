// Translated from solution.cpp.

var n: dynamic;

var s: dynamic;

func main()
{
  read(n, s, s);
  write((if ((s == "week")) (if (((n == 6) || (n == 5))) 53 else 52) else (if ((n == 31)) 7 else (if ((n == 30)) 11 else 12))));
  return 0;
}
