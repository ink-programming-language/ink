// Translated from solution.cpp.

var N = (5e6 + 5);

var mod = (1e9 + 7);

var t: dynamic;

var n: dynamic;

var x: dynamic;

var a = cpp_array(N);

var s: dynamic;

func main()
{
  read(t);
  while (cpp_update(t, "--"))
  {
    read(x, s);
    {
      var i = 1;
      while ((i <= s.length()))
      {
        a[i] = (s[(i - 1)] - 48);
        i += 1;
      }
    }
    var len = s.length();
    var kt = 0;
    {
      var i = 1;
      while ((i <= x))
      {
        if ((kt == 0))
        {
          var luu = len;
          {
            var j = 1;
            while ((j <= (a[i] - 1)))
            {
              {
                var k = (i + 1);
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
          var tmp = ((((len - i) + (mod * mod))) % mod);
          len = (((i + (((a[i] * tmp)) % mod))) % mod);
        }
        i += 1;
      }
    }
    write(len, "\n");
  }
}
