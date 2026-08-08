// Translated from solution.cpp.

var mod = (1e9 + 7);

class Num
{
  var a: dynamic;
  func Num(a: dynamic = 0)
  {
      this->a = cpp_construct(a);
    }
  func operator_add(cpp_name: dynamic)
  {
      var ret = (a + cpp_name.a);
      return if ((ret < mod)) Num(ret) else Num((ret - mod));
    }
  func operator_subtract(cpp_name: dynamic)
  {
      return ((*this) + Num((mod - cpp_name.a)));
    }
  func operator_multiply(cpp_name: dynamic)
  {
      return Num(cpp_cast((((cpp_cast(a) * cpp_name.a) % mod))));
    }
  func operator_add_assign(cpp_name: dynamic)
  {
      (*this) = ((*this) + cpp_name);
    }
}

var pw2: dynamic;

var n: dynamic;

var x: dynamic;

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n);
  x.assign(n, 0);
  {
    var i = 0;
    while ((i < n))
    {
      read(x[i]);
      i += 1;
    }
  }
  sort((x).begin(), (x).end());
  pw2.assign(n, Num(1));
  {
    var i = 1;
    while ((i < (cpp_cast((pw2).size()))))
    {
      pw2[i] = (pw2[(i - 1)] * Num(2));
      i += 1;
    }
  }
  var ans: dynamic;
  {
    var i = 0;
    while (((i + 1) < n))
    {
      ans += ((((pw2[(i + 1)] - Num(1))) * ((pw2[((n - i) - 1)] - 1))) * Num((x[(i + 1)] - x[i])));
      i += 1;
    }
  }
  write(ans.a, cpp_char("\n"));
  return 0;
}
