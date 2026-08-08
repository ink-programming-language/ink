// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func main(argument_0: dynamic)
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic;
  var ans = 0;
  read(N);
  var s = cpp_array(N);
  rep(i, N);
  read(s[i]);
  {
    var k = 0;
    while ((k < N))
    {
      var ok = 1;
      {
        var i = 0;
        while ((i < N))
        {
          {
            var j = 0;
            while ((j < N))
            {
              if (((((i + k)) % N) == j))
              {
                j += 1;
                continue;
              }
              if ((s[i][((((j - k) + N)) % N)] == s[j][((((i - k) + N)) % N)]))
              {
              } else
              {
                ok = 0;
                break;
              }
              j += 1;
            }
          }
          if ((!ok))
          {
            break;
          }
          i += 1;
        }
      }
      if (ok)
      {
        ans += 1;
      }
      k += 1;
    }
  }
  write((ans * N), "\n");
}
