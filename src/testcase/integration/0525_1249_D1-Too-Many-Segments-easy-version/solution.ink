// Translated from solution.cpp.

var arr = cpp_array(200005);

var tree = cpp_array((4 * 200005));

var lazy = cpp_array((4 * 200005));

class info
{
  var l: dynamic;
  var r: dynamic;
  var i: dynamic;
}

var vec: dynamic;

func comp(a: dynamic, b: dynamic)
{
  if ((a.r != b.r))
  {
    return (a.r < b.r);
  }
  return (a.l < b.l);
}

func update(node: dynamic, a: dynamic, b: dynamic, i: dynamic, j: dynamic, value: dynamic)
{
  if ((lazy[node] != 0))
  {
    tree[node] += lazy[node];
    if ((a != b))
    {
      lazy[(node * 2)] += lazy[node];
      lazy[((node * 2) + 1)] += lazy[node];
    }
    lazy[node] = 0;
  }
  if ((((a > b) || (a > j)) || (b < i)))
  {
    return;
  }
  if (((a >= i) && (b <= j)))
  {
    tree[node] += value;
    if ((a != b))
    {
      lazy[(node * 2)] += value;
      lazy[((node * 2) + 1)] += value;
    }
    return;
  }
  var mid = (((a + b)) / 2);
  update((node * 2), a, mid, i, j, value);
  update((1 + (node * 2)), (1 + mid), b, i, j, value);
  tree[node] = max(tree[(node * 2)], tree[((node * 2) + 1)]);
}

func query(node: dynamic, a: dynamic, b: dynamic, i: dynamic, j: dynamic)
{
  if ((((a > b) || (a > j)) || (b < i)))
  {
    return 0;
  }
  if ((lazy[node] != 0))
  {
    tree[node] += lazy[node];
    if ((a != b))
    {
      lazy[(node * 2)] += lazy[node];
      lazy[((node * 2) + 1)] += lazy[node];
    }
    lazy[node] = 0;
  }
  if (((a >= i) && (b <= j)))
  {
    return tree[node];
  }
  var mid = (((a + b)) / 2);
  var q1 = query((node * 2), a, mid, i, j);
  var q2 = query((1 + (node * 2)), (1 + mid), b, i, j);
  var res = max(q1, q2);
  return res;
}

func main()
{
  var i: dynamic;
  var n: dynamic;
  var k: dynamic;
  var a: dynamic;
  var b: dynamic;
  var mx = -1;
  scanf("%d %d", (&n), (&k));
  {
    i = 0;
    while ((i < n))
    {
      scanf("%d %d", (&a), (&b));
      mx = max(mx, b);
      vec.push_back([a, b, (i + 1)]);
      i += 1;
    }
  }
  sort((vec).begin(), (vec).end(), comp);
  var q: dynamic;
  {
    i = 0;
    while ((i < n))
    {
      var l = vec[i].l;
      var r = vec[i].r;
      var mxv = query(1, 0, (mx - 1), (l - 1), (r - 1));
      if ((mxv < k))
      {
        update(1, 0, (mx - 1), (l - 1), (r - 1), 1);
      } else
      {
        q.push(vec[i].i);
      }
      i += 1;
    }
  }
  printf("%d\n", q.size());
  while ((!q.empty()))
  {
    printf("%d ", q.front());
    q.pop();
  }
}
