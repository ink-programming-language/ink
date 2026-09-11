// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

func main(argument_0: dynamic) -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(N);
  var s: dynamic = cpp_array(N);
  rep(i, N);
  read(s[i]);
  {
    var k: dynamic = 0;
    while ((k < N))
    {
      var ok: dynamic = 1;
      {
        var i: dynamic = 0;
        while ((i < N))
        {
          {
            var j: dynamic = 0;
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
