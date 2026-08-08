// Translated from solution.cpp.

var mod = (1e9 + 7);

func main()
{
  var N: dynamic;
  read(N);
  var ans = 0;
  {
    var i = 0;
    while ((i < N))
    {
      read(A[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 60))
    {
      var t = 0;
      {
        var j = 0;
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
