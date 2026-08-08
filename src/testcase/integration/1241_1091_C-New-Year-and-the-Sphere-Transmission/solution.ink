// Translated from solution.cpp.

var ans: dynamic;

func foo(n: dynamic, k: dynamic)
{
  var num = (n / k);
  var ret = (((num * ((num - 1)))) / 2);
  ret *= k;
  ret += num;
  return ret;
}

func main()
{
  var n: dynamic;
  scanf("%lld", (&n));
  var factors: dynamic;
  {
    var i = 1;
    while (((i * i) <= n))
    {
      if (((n % i) == 0))
      {
        factors.push_back(i);
        factors.push_back((n / i));
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < factors.size()))
    {
      ans.insert(foo(n, factors[i]));
      i += 1;
    }
  }
  {
    var it = (ans).begin();
    while ((it != (ans).end()))
    {
      printf("%lld ", (*it));
      it += 1;
    }
  }
  printf("\n");
  return 0;
}
