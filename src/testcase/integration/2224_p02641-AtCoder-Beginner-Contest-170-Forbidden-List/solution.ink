// Translated from solution.cpp.

var p: dynamic = cpp_array(105);

var x: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(x, n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(t);
      p[t] = 1;
      i += 1;
    }
  }
  var min: dynamic = 1005;
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
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
