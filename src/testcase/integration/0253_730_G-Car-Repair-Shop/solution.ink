// Translated from solution.cpp.

var maxn: dynamic = 209;

class node
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func node() -> dynamic
  {
    }
  func node(l: dynamic, r: dynamic) -> dynamic
  {
      self->l = cpp_construct(l);
      self->r = cpp_construct(r);
    }
  func operator_less(R: dynamic) -> dynamic
  {
      return (l < R.l);
    }
}

var S: dynamic = cpp_uninitialized();

var it: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  S.insert(node(1, 2e9));
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var l: dynamic = cpp_uninitialized();
      var len: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      scanf("%d%d", (&l), (&len));
      r = ((l + len) - 1);
      var have: dynamic = 0;
      {
        it = S.begin();
        while ((it != S.end()))
        {
          var L: dynamic = ((*it)).l;
          var R: dynamic = ((*it)).r;
          if (((L <= l) && (R >= r)))
          {
            S.erase(it);
            printf("%d %d\n", l, r);
            if ((l != L))
            {
              S.insert(node(L, (l - 1)));
            }
            if ((r != R))
            {
              S.insert(node((r + 1), R));
            }
            have = 1;
            break;
          }
          it += 1;
        }
      }
      if (have)
      {
        i += 1;
        continue;
      }
      {
        it = S.begin();
        while ((it != S.end()))
        {
          var L: dynamic = ((*it)).l;
          var R: dynamic = ((*it)).r;
          if ((((R - L) + 1) >= len))
          {
            l = L;
            r = ((L + len) - 1);
            S.erase(it);
            printf("%d %d\n", l, r);
            if ((l != L))
            {
              S.insert(node(L, (l - 1)));
            }
            if ((r != R))
            {
              S.insert(node((r + 1), R));
            }
            break;
          }
          it += 1;
        }
      }
      i += 1;
    }
  }
}
