// Translated from solution.cpp.

class edge
{
  var to: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var E: dynamic = cpp_array(6010);

var ne: dynamic = E;

var first: dynamic = cpp_uninitialized();

func link(a: dynamic, b: dynamic) -> dynamic
{
  (*ne) = [b, first[a]];
  first[a] = cpp_update(ne, "++");
}

var C: dynamic = cpp_array(3010);

var cnt: dynamic = cpp_uninitialized();

var tag: dynamic = cpp_array(3010);

func dfs1(i: dynamic, f: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  tag[i] = 1;
  {
    var e: dynamic = first[i];
    while (e)
    {
      if ((e->to != f))
      {
        if (tag[e->to])
        {
          return cpp_comma(cpp_assign(C[cpp_update(cnt, "++")], "=", i), e->to);
        } else if (((cpp_assign(t, "=", dfs1(e->to, i))) > -1))
        {
          return  (((cpp_assign(C[cpp_update(cnt, "++")], "=", i)) == t)) ? -1 : t;
        } else if ((t == -1))
        {
          return cpp_comma(cpp_assign(tag[i], "=", 0), -1);
        }
      }
      e = e->next;
    }
  }
  tag[i] = 0;
  return -2;
}

func dfs2(i: dynamic, f: dynamic, d: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  var s: dynamic = (((1.0 / ((d + l))) + (1.0 / ((d + r)))) - (1.0 / (((d + l) + r))));
  {
    var e: dynamic = first[i];
    while (e)
    {
       (((e->to != f) && (((!tag[i]) || (!tag[e->to]))))) ? cpp_assign(s, "+=", dfs2(e->to, i, (d + 1), l, r)) : 1;
      e = e->next;
    }
  }
  if ((tag[i] && (((f == -1) || (!tag[f])))))
  {
    var k: dynamic = 0;
    while ((C[k] != i))
    {
      k += 1;
    }
    r = (cnt - 2);
    {
      var j: dynamic = (((k + 1)) % cnt);
      while ((j != k))
      {
        s += dfs2(C[j], i, (d + 1), l, r);
        j = (((j + 1)) % cnt);
        l += 1;
        r -= 1;
      }
    }
  }
  return s;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d%d", (&a), (&b));
      link(a, b);
      link(b, a);
      i += 1;
    }
  }
  dfs1(0, -1);
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      ans += dfs2(i, -1, 1, 0, 0);
      i += 1;
    }
  }
  printf("%.12lf\n", ans);
}
