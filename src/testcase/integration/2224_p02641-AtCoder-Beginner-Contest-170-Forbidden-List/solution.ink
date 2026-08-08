// Translated from solution.cpp.

var p = cpp_array(105);

var x: dynamic;

var n: dynamic;

var t: dynamic;

func main()
{
  read(x, n);
  {
    var i = 0;
    while ((i < n))
    {
      read(t);
      p[t] = 1;
      i += 1;
    }
  }
  var min = 1005;
  var ans = 0;
  {
    var i = 0;
    while ((i <= 101))
    {
      if ((((!p[i])) && (abs((i - x)) < min)))
      {
        min = abs((i - x));
        ans = i;
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
