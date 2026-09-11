// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var MAX_N: dynamic = (2 * 100000);

func main() -> dynamic
{
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n, k);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(vec[i]);
        i += 1;
      }
    }
    var mini: dynamic = 2000000000;
    var res: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < (n - k)))
      {
        var check: dynamic = (vec[(k + i)] - vec[i]);
        if (((check % 2) != 0))
        {
          if ((((check / 2) + 1) < mini))
          {
            mini = (((check) / 2) + 1);
            res = (vec[i] + mini);
          }
        } else
        {
          if (((((vec[(k + i)] - vec[i])) / 2) < mini))
          {
            mini = (((vec[(k + i)] - vec[i])) / 2);
            res = (vec[i] + mini);
          }
        }
        i += 1;
      }
    }
    write(res, "\n");
  }
}
