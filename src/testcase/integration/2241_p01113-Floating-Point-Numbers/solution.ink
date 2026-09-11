// Translated from solution.cpp.

var ll: dynamic = dynamic;

func fornum(A: dynamic, B: dynamic, C: dynamic) -> dynamic
{
  cpp_macro("for(A=B;A<C;A++)");
}

var mp: dynamic = cpp_expression("#include<");

var pii: dynamic = cpp_expression("#include<bits");

var pll: dynamic = cpp_expression("#include<bi");

var nxtr: dynamic = cpp_expression("#include<");

var n: dynamic = cpp_uninitialized();

var bs: dynamic = cpp_array(255);

var mk: dynamic = cpp_array(11, 11);

var kt: dynamic = cpp_array(11);

var ans: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%lld", (&n));
  while (n)
  {
    scanf("%s", bs);
    a = 1;
    b = 0;
    ans = 1;
    fornum(i, 0, 52);
    {
      ans *= 2;
      a *= 2;
      ans +=  ((bs[i] == cpp_char("1"))) ? 1 : 0;
      a +=  ((bs[i] == cpp_char("1"))) ? 1 : 0;
    }
    i = 0;
    while (true)
    {
      var aa: dynamic = (nxtr - ans);
      var ai: dynamic = (((aa / a) + ( ((aa % a)) ? 1 : 0)));
      if (((ai + i) <= n))
      {
        ans += (ai * a);
      } else
      {
        ans += (((n - i)) * a);
        break;
      }
      i += ai;
      b += 1;
      ans /= 2;
      a /= 2;
      if ((a == 0))
      {
        break;
      }
    }
    fornum(i, 0, 12);
    {
      printf("%lld",  (((((b >> ((11 - i)))) & 1))) ? 1 : 0);
    }
    fornum(i, 0, 52);
    {
      printf("%lld",  (((((ans >> ((51 - i)))) & 1))) ? 1 : 0);
    }
    printf("\n");
    scanf("%lld", (&n));
  }
  return 0;
}
