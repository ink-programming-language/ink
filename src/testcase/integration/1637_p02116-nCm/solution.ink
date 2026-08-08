// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int (i)=0;(i)<(int)(n);++(i))");
}

var pb = cpp_expression("#include");

func ct2(n: dynamic)
{
  var r = 0;
  while (((n > 0) && ((n % 2) == 0)))
  {
    n /= 2;
    r += 1;
  }
  return r;
}

func pw(x: dynamic, n: dynamic)
{
  var r = 1;
  rep(i, n) *= x;
  return r;
}

func num(n: dynamic)
{
  var ct = 0;
  rep(i, (n + 1));
  {
    ct += ct2(i);
  }
  return ct;
}

func C(n: dynamic, m: dynamic)
{
  var p = num(n);
  var q = (num(m) + num((n - m)));
  return (p - q);
}

func test()
{
  {
    var i = 3;
    while ((i <= 100000))
    {
      printf(" i %d\n", i);
      {
        var j = 2;
        while ((j <= i))
        {
          if ((C(i, j) > 0))
          {
            assert((j == pw(2, ct2((i + 1)))));
            break;
          }
          j += 2;
        }
      }
      i += 2;
    }
  }
  printf(" END\n");
}

func main()
{
  var n: dynamic;
  read(n);
  write(pw(2, ct2((n + 1))), "\n");
  return 0;
}
