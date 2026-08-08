// Translated from solution.cpp.

var c = cpp_array(1010);

var n: dynamic;

var as_cpp: dynamic;

var nw: dynamic;

func main()
{
  var i: dynamic;
  var j: dynamic;
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
