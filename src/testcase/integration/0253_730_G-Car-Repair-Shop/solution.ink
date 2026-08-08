// Translated from solution.cpp.

var maxn = 209;

class node
{
  var l: dynamic;
  var r: dynamic;
  func node()
  {
    }
  func node(l: dynamic, r: dynamic)
  {
      this->l = cpp_construct(l);
      this->r = cpp_construct(r);
    }
  func operator_less(R: dynamic)
  {
      return (l < R.l);
    }
}

var S: dynamic;

var it: dynamic;

func main()
{
  S.insert(node(1, 2e9));
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      var l: dynamic;
      var len: dynamic;
      var r: dynamic;
      scanf("%d%d", (&l), (&len));
      r = ((l + len) - 1);
      var have = 0;
      {
        it = S.begin();
        while ((it != S.end()))
        {
          var L = ((*it)).l;
          var R = ((*it)).r;
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
          var L = ((*it)).l;
          var R = ((*it)).r;
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
