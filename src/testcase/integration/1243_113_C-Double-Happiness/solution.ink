// Translated from solution.cpp.

var prime: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  read(l, r);
  var res: dynamic = ((l <= 2) && (r >= 2));
  prime.set();
  prime[0] = false;
  prime[1] = false;
  {
    var i: dynamic = 3;
    while (((i * i) <= r))
    {
      if (prime[i])
      {
        {
          var j: dynamic = (i * i);
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
  var f: dynamic = (((l / 4) * 4) + 1);
  if ((f < l))
  {
    f += 4;
  }
  {
    var i: dynamic = f;
    while ((i <= r))
    {
      res += prime[i];
      i += 4;
    }
  }
  printf("%d\n", res);
  return 0;
}
