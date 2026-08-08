// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var x: dynamic;

var k: dynamic;

var t: dynamic;

var MAX_N = (2 * 100000);

func main()
{
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n, k);
    {
      var i = 0;
      while ((i < n))
      {
        read(vec[i]);
        i += 1;
      }
    }
    var mini = 2000000000;
    var res = 0;
    {
      var i = 0;
      while ((i < (n - k)))
      {
        var check = (vec[(k + i)] - vec[i]);
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
