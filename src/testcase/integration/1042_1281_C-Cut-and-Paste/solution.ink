// Translated from solution.cpp.

var N: dynamic = (5e6 + 5);

var mod: dynamic = (1e9 + 7);

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N);

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(t);
  while (cpp_update(t, "--"))
  {
    read(x, s);
    {
      var i: dynamic = 1;
      while ((i <= s.length()))
      {
        a[i] = (s[(i - 1)] - 48);
        i += 1;
      }
    }
    var len: dynamic = s.length();
    var kt: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= x))
      {
        if ((kt == 0))
        {
          var luu: dynamic = len;
          {
            var j: dynamic = 1;
            while ((j <= (a[i] - 1)))
            {
              {
                var k: dynamic = (i + 1);
                while ((k <= luu))
                {
                  len += 1;
                  a[len] = a[k];
                  k += 1;
                }
              }
              j += 1;
            }
          }
          if ((len >= x))
          {
            kt = 1;
          }
        } else
        {
          var tmp: dynamic = ((((len - i) + (mod * mod))) % mod);
          len = (((i + (((a[i] * tmp)) % mod))) % mod);
        }
        i += 1;
      }
    }
    write(len, "\n");
  }
}
