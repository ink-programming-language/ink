// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_array(55);
  var r: dynamic = cpp_array(55);
  var t: dynamic = cpp_array(55);
  var max: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  while (true)
  {
    read(n);
    if ((n == 0))
    {
      break;
    }
    read(y);
    max = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(b[i]);
        read(r[b[i]], t[b[i]]);
        if ((t[b[i]] == 1))
        {
          q = (100 + (y * r[b[i]]));
        } else
        {
          var in_cpp: dynamic = (100 + r[b[i]]);
          q = in_cpp;
          {
            var s: dynamic = 0;
            while ((s < (y - 1)))
            {
              q *= in_cpp;
              s += 1;
            }
          }
          {
            var s: dynamic = 0;
            while ((s < (y - 1)))
            {
              q /= 100;
              s += 1;
            }
          }
        }
        if ((max < q))
        {
          max = q;
          ans = b[i];
        }
        i += 1;
      }
    }
    write(ans, "\n");
  }
  return 0;
}
