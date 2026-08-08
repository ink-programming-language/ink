// Translated from solution.cpp.

var prime: dynamic;

func main()
{
  var l: dynamic;
  var r: dynamic;
  read(l, r);
  var res = ((l <= 2) && (r >= 2));
  prime.set();
  prime[0] = false;
  prime[1] = false;
  {
    var i = 3;
    while (((i * i) <= r))
    {
      if (prime[i])
      {
        {
          var j = (i * i);
          while ((j <= r))
          {
            prime[j] = false;
            j += ((i << 1));
          }
        }
      }
      i += 2;
    }
  }
  var f = (((l / 4) * 4) + 1);
  if ((f < l))
  {
    f += 4;
  }
  {
    var i = f;
    while ((i <= r))
    {
      res += prime[i];
      i += 4;
    }
  }
  printf("%d\n", res);
  return 0;
}
