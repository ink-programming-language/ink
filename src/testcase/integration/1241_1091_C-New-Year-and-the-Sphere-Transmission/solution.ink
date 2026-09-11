// Translated from solution.cpp.

var ans: dynamic = cpp_uninitialized();

func foo(n: dynamic, k: dynamic) -> dynamic
{
  var num: dynamic = (n / k);
  var ret: dynamic = (((num * ((num - 1)))) / 2);
  ret *= k;
  ret += num;
  return ret;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%lld", (&n));
  var factors: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
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
    var i: dynamic = 0;
    while ((i < factors.size()))
    {
      ans.insert(foo(n, factors[i]));
      i += 1;
    }
  }
  {
    var it: dynamic = (ans).begin();
    while ((it != (ans).end()))
    {
      printf("%lld ", (*it));
      it += 1;
    }
  }
  printf("\n");
  return 0;
}
