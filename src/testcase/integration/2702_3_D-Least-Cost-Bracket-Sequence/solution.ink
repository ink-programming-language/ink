// Translated from solution.cpp.

var str = cpp_array(50010);

var cnt = 0;

var a: dynamic;

var b: dynamic;

var len: dynamic;

var ans: dynamic;

class cost
{
  var p: dynamic;
  var v: dynamic;
  func operator_less(a: dynamic)
  {
      return (v < a.v);
    }
}

var q: dynamic;

func make(p: dynamic, v: dynamic)
{
  return [p, v];
}

func main()
{
  scanf("%s", str);
  len = strlen(str);
  ans = 0;
  {
    var i = 0;
    while ((i < len))
    {
      cnt += (str[i] == cpp_char("("));
      cnt -= ((str[i] == cpp_char(")")) || (str[i] == cpp_char("?")));
      if ((str[i] == cpp_char("?")))
      {
        scanf("%d%d", (&a), (&b));
        q.push(make(i, (b - a)));
        ans += b;
        str[i] = cpp_char(")");
      }
      if (((cnt < 0) && q.empty()))
      {
        ans = -1;
        break;
      }
      if ((cnt < 0))
      {
        var top = q.top();
        q.pop();
        ans = (ans - top.v);
        str[top.p] = cpp_char("(");
        cnt += 2;
      }
      i += 1;
    }
  }
  if ((cnt > 0))
  {
    ans = -1;
  }
  printf("%lld\n", ans);
  if ((ans != -1))
  {
    printf("%s\n", str);
  }
  return 0;
}
