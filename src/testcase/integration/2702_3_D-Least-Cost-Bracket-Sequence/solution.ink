// Translated from solution.cpp.

var str: dynamic = cpp_array(50010);

var cnt: dynamic = 0;

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var len: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

class cost
{
  var p: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  func operator_less(a: dynamic) -> dynamic
  {
      return (v < a.v);
    }
}

var q: dynamic = cpp_uninitialized();

func make(p: dynamic, v: dynamic) -> dynamic
{
  return [p, v];
}

func main() -> dynamic
{
  scanf("%s", str);
  len = strlen(str);
  ans = 0;
  {
    var i: dynamic = 0;
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
        var top: dynamic = q.top();
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
