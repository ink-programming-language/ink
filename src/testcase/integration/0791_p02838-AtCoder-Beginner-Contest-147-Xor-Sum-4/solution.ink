// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(A[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 60))
    {
      var t: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j < N))
        {
          if ((A[j] & ((1 << i))))
          {
            t += 1;
          }
          j += 1;
        }
      }
      ans = (((ans + ((((((t * ((N - t)))) % mod)) * ((((1 << i)) % mod)))))) % mod);
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
