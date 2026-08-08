// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var y: dynamic;
  var b = cpp_array(55);
  var r = cpp_array(55);
  var t = cpp_array(55);
  var max: dynamic;
  var ans: dynamic;
  var q: dynamic;
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
      var i = 0;
      while ((i < n))
      {
        read(b[i]);
        read(r[b[i]], t[b[i]]);
        if ((t[b[i]] == 1))
        {
          q = (100 + (y * r[b[i]]));
        } else
        {
          var in_cpp = (100 + r[b[i]]);
          q = in_cpp;
          {
            var s = 0;
            while ((s < (y - 1)))
            {
              q *= in_cpp;
              s += 1;
            }
          }
          {
            var s = 0;
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
