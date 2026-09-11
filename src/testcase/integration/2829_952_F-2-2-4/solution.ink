// Translated from solution.cpp.

var c: dynamic = cpp_array(1010);

var n: dynamic = cpp_uninitialized();

var as_cpp: dynamic = cpp_uninitialized();

var nw: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  scanf("%d%s", (&as_cpp), c);
  n = strlen(c);
  {
    i = 0;
    while ((i < n))
    {
      nw = (c[i] - cpp_char("0"));
      j = (i + 1);
      while (isdigit(c[j]))
      {
        nw = (((nw * 10) + c[j]) - cpp_char("0"));
        j += 1;
      }
      if ((c[i] == cpp_char("+")))
      {
        as_cpp += nw;
      } else
      {
        as_cpp -= nw;
      }
      i = j;
    }
  }
  write(as_cpp);
  return 0;
}
