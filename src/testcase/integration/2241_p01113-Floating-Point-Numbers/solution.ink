// Translated from solution.cpp.

var ll = dynamic;

func fornum(A: dynamic, B: dynamic, C: dynamic)
{
  cpp_macro("for(A=B;A<C;A++)");
}

var mp = cpp_expression("#include<");

var pii = cpp_expression("#include<bits");

var pll = cpp_expression("#include<bi");

var nxtr = cpp_expression("#include<");

var n: dynamic;

var bs = cpp_array(255);

var mk = cpp_array(11, 11);

var kt = cpp_array(11);

var ans: dynamic;

var a: dynamic;

var b: dynamic;

var i: dynamic;

var j: dynamic;

var k: dynamic;

func main()
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
      ans += if ((bs[i] == cpp_char("1"))) 1 else 0;
      a += if ((bs[i] == cpp_char("1"))) 1 else 0;
    }
    i = 0;
    while (true)
    {
      var aa = (nxtr - ans);
      var ai = (((aa / a) + (if ((aa % a)) 1 else 0)));
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
      printf("%lld", if (((((b >> ((11 - i)))) & 1))) 1 else 0);
    }
    fornum(i, 0, 52);
    {
      printf("%lld", if (((((ans >> ((51 - i)))) & 1))) 1 else 0);
    }
    printf("\n");
    scanf("%lld", (&n));
  }
  return 0;
}
