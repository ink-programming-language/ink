// Translated from solution.cpp.

var tree = cpp_array(2, 1000000);

var ar = cpp_array(1000000);

var a: dynamic;

var b: dynamic;

func query(x: dynamic, l: dynamic, r: dynamic, s: dynamic, e: dynamic, k: dynamic)
{
  if ((((l > e) || (r < s)) || (s > e)))
  {
    return 0;
  }
  if (((l >= s) && (r <= e)))
  {
    return tree[x][k];
  } else
  {
    return (query((x * 2), l, (((l + r)) / 2), s, e, k) + query(((x * 2) + 1), ((((l + r)) / 2) + 1), r, s, e, k));
  }
}

func upd(x: dynamic, l: dynamic, r: dynamic, v: dynamic)
{
  if (((v > r) || (v < l)))
  {
    return;
  }
  if (((l == r) && (v == l)))
  {
    tree[x][0] = min(ar[v], (b + 0));
    tree[x][1] = min(ar[v], (a + 0));
  } else
  {
    upd((x * 2), l, (((l + r)) / 2), v);
    upd(((x * 2) + 1), ((((l + r)) / 2) + 1), r, v);
    tree[x][0] = (tree[(x * 2)][0] + tree[((x * 2) + 1)][0]);
    tree[x][1] = (tree[(x * 2)][1] + tree[((x * 2) + 1)][1]);
  }
}

func main()
{
  var n: dynamic;
  var k: dynamic;
  var q: dynamic;
  read(n, k, a, b, q);
  while (cpp_update(q, "--"))
  {
    var typ: dynamic;
    scanf("%d", (&typ));
    if ((typ == 1))
    {
      var d: dynamic;
      var x: dynamic;
      scanf("%d%d", (&d), (&x));
      ar[d] += x;
      upd(1, 1, n, d);
    } else
    {
      var p: dynamic;
      scanf("%d", (&p));
      printf("%I64d\n", (query(1, 1, n, 1, (p - 1), 0) + query(1, 1, n, (p + k), n, 1)));
    }
  }
}
