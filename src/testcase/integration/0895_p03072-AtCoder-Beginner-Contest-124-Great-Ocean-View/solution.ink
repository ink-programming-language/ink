// Translated from solution.cpp.

func main()
{
  var N: dynamic;
  read(N);
  var maxi = 0;
  var ans = 0;
  {
    var i = 0;
    while ((i < N))
    {
      var H: dynamic;
      read(H);
      if ((H >= maxi))
      {
        maxi = H;
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
