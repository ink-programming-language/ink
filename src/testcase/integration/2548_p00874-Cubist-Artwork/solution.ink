// Translated from solution.cpp.

var W: dynamic = cpp_uninitialized();

var D: dynamic = cpp_uninitialized();

var hw: dynamic = cpp_array(21);

var hd: dynamic = cpp_array(21);

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  while (cpp_comma(((cin >> W) >> D), W))
  {
    {
      var i: dynamic = 0;
      while ((i < 21))
      {
        hw[i] = 0;
        hd[i] = 0;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < W))
      {
        read(h);
        hw[h] += 1;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < D))
      {
        read(h);
        hd[h] += 1;
        i += 1;
      }
    }
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i <= 20))
      {
        ans += (max(hd[i], hw[i]) * i);
        i += 1;
      }
    }
    write(ans, "\n");
  }
}
