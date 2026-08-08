// Translated from solution.cpp.

var eps = 1e-8;

var INF = (((1 << 30)) - 1);

var mod = 190102321;

var MAXN = 200010;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var s = cpp_array(MAXN);

class Hash
{
  var H: dynamic = cpp_array(MAXN);
  var B: dynamic = cpp_array(MAXN);
  var TH: dynamic = cpp_array(MAXN, 10);
  func init(s: dynamic, P: dynamic, len: dynamic)
  {
      H[0] = 0;
      B[0] = 1;
      {
        var i = 1;
        while ((i <= len))
        {
          B[i] = ((B[(i - 1)] * P) % mod);
          H[i] = (((((H[(i - 1)] * P) + s[i]) - cpp_char("0"))) % mod);
          i += 1;
        }
      }
      {
        var v = 0;
        while ((v < 10))
        {
          TH[v][0] = 0;
          {
            var i = 1;
            while ((i <= len))
            {
              TH[v][i] = (((((TH[v][(i - 1)] * P) % mod) + v)) % mod);
              i += 1;
            }
          }
          v += 1;
        }
      }
    }
  func get(l: dynamic, r: dynamic)
  {
      return ((((((H[r] - (H[(l - 1)] * B[((r - l) + 1)]))) % mod) + mod)) % mod);
    }
  func get_fixed(num: dynamic, len: dynamic)
  {
      return TH[num][len];
    }
}

var H: dynamic;

var t = cpp_array((MAXN << 2));

var tag = cpp_array((MAXN << 2));

var L = cpp_array((MAXN << 2));

var R = cpp_array((MAXN << 2));

func Build(p: dynamic, l: dynamic, r: dynamic)
{
  t[p] = H.get(l, r);
  tag[p] = -1;
  L[p] = l;
  R[p] = r;
  if ((l == r))
  {
    return;
  }
  var mid = ((cpp_cast((+(((r) - (l))))) / 2));
  Build((p << 1), l, mid);
  Build(((p << 1) | 1), (mid + 1), r);
}

func Push_down(p: dynamic)
{
  if ((tag[p] != -1))
  {
    tag[(p << 1)] = cpp_assign(tag[((p << 1) | 1)], "=", tag[p]);
    t[(p << 1)] = H.get_fixed(tag[(p << 1)], ((R[(p << 1)] - L[(p << 1)]) + 1));
    t[((p << 1) | 1)] = H.get_fixed(tag[((p << 1) | 1)], ((R[((p << 1) | 1)] - L[((p << 1) | 1)]) + 1));
    tag[p] = -1;
  }
}

func Push_up(p: dynamic)
{
  var len_r = ((R[((p << 1) | 1)] - L[((p << 1) | 1)]) + 1);
  t[p] = (((((t[(p << 1)] * H.B[len_r]) % mod) + t[((p << 1) | 1)])) % mod);
}

func Update(a: dynamic, b: dynamic, c: dynamic, p: dynamic, l: dynamic, r: dynamic)
{
  if (((a <= l) && (r <= b)))
  {
    t[p] = H.get_fixed(c, ((r - l) + 1));
    tag[p] = c;
    return;
  }
  Push_down(p);
  var mid = ((cpp_cast((+(((r) - (l))))) / 2));
  if ((a <= mid))
  {
    Update(a, b, c, (p << 1), l, mid);
  }
  if ((b > mid))
  {
    Update(a, b, c, ((p << 1) | 1), (mid + 1), r);
  }
  Push_up(p);
}

func Query(a: dynamic, b: dynamic, p: dynamic, l: dynamic, r: dynamic)
{
  if (((a <= l) && (r <= b)))
  {
    return t[p];
  }
  Push_down(p);
  var mid = ((cpp_cast((+(((r) - (l))))) / 2));
  var res = 0;
  var L: dynamic;
  var R: dynamic;
  if ((a <= mid))
  {
    res = Query(a, b, (p << 1), l, mid);
  }
  if ((b > mid))
  {
    R = Query(a, b, ((p << 1) | 1), (mid + 1), r);
    if ((a <= mid))
    {
      res = (((((res * H.B[(min(r, b) - mid)]) % mod) + R)) % mod);
    } else
    {
      res = R;
    }
  }
  return res;
}

func main()
{
  scanf("%d%d%d", (&n), (&m), (&k));
  scanf("%s", (s + 1));
  var len = strlen((s + 1));
  H.init(s, 10, len);
  Build(1, 1, len);
  {
    var i = 1;
    while ((i <= (m + k)))
    {
      var a: dynamic;
      var b: dynamic;
      var c: dynamic;
      var d: dynamic;
      scanf("%d%d%d%d", (&a), (&b), (&c), (&d));
      if ((a == 1))
      {
        Update(b, c, d, 1, 1, len);
      } else
      {
        if ((d == ((c - b) + 1)))
        {
          printf("YES\n");
        } else if ((Query(b, (c - d), 1, 1, len) == Query((b + d), c, 1, 1, len)))
        {
          printf("YES\n");
        } else
        {
          printf("NO\n");
        }
      }
      i += 1;
    }
  }
  return 0;
}
