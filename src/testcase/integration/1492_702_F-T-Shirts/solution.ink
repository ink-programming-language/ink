// Translated from solution.cpp.

func read(n: dynamic)
{
  var x = 0;
  var f = 1;
  var ch = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((((x << 1)) + ((x << 3))) + ch) - cpp_char("0"));
    ch = getchar();
  }
  n = (x * f);
}

var outputarray = cpp_array(20);

func write(k: dynamic)
{
  var num = 0;
  if ((k < 0))
  {
    putchar(cpp_char("-"));
    k = (-k);
  }
  while (true)
  {
    outputarray[cpp_update(num, "++")] = (k % 10);
    k /= 10;
    if (!((k)))
    {
      break;
    }
  }
  while (num)
  {
    putchar((outputarray[cpp_update(num, "--")] + cpp_char("0")));
  }
}

var maxn = 200001;

var n: dynamic;

var m: dynamic;

var ans = cpp_array(maxn);

class T
{
  var p: dynamic;
  var q: dynamic;
  func operator_less(b: dynamic)
  {
      return if ((q != b.q)) (q > b.q) else (p < b.p);
    }
}

var s = cpp_array(maxn);

class node
{
  var key: dynamic;
  var id: dynamic;
  var l1: dynamic;
  var l2: dynamic;
  var s1: dynamic;
  var s2: dynamic;
  var l: dynamic;
  var r: dynamic;
  func node(w: dynamic, id: dynamic)
  {
      this->s1 = cpp_construct(w);
      this->id = cpp_construct(id);
      key = rand();
      l1 = cpp_assign(l2, "=", cpp_assign(s2, "=", 0));
      l = cpp_assign(r, "=", null);
    }
  func pushdown()
  {
      if (l1)
      {
        if ((l != null))
        {
          l->l1 += l1;
          l->s1 += l1;
        }
        if ((r != null))
        {
          r->l1 += l1;
          r->s1 += l1;
        }
        l1 = 0;
      }
      if (l2)
      {
        if ((l != null))
        {
          l->l2 += l2;
          l->s2 += l2;
        }
        if ((r != null))
        {
          r->l2 += l2;
          r->s2 += l2;
        }
        l2 = 0;
      }
    }
}

func split(root: dynamic, a: dynamic, b: dynamic, v: dynamic)
{
  if ((root == null))
  {
    a = cpp_assign(b, "=", null);
    return;
  }
  root->pushdown();
  if ((root->s1 >= v))
  {
    b = root;
    split(root->l, a, b->l, v);
  } else
  {
    a = root;
    split(root->r, a->r, b, v);
  }
}

func merge(a: dynamic, b: dynamic)
{
  if ((a == null))
  {
    return b;
  }
  if ((b == null))
  {
    return a;
  }
  if ((a->key < b->key))
  {
    a->pushdown();
    a->r = merge(a->r, b);
    return a;
  }
  b->pushdown();
  b->l = merge(a, b->l);
  return b;
}

func insert(a: dynamic, b: dynamic)
{
  if ((a == null))
  {
    return b;
  }
  var l: dynamic;
  var r: dynamic;
  split(a, l, r, b->s1);
  return merge(l, merge(b, r));
}

func left(root: dynamic)
{
  while ((root->l != null))
  {
    root->pushdown();
    root = root->l;
  }
  return root;
}

func right(root: dynamic)
{
  while ((root->r != null))
  {
    root->pushdown();
    root = root->r;
  }
  return root;
}

func updata(root: dynamic, v: dynamic, w: dynamic)
{
  root->l1 += v;
  root->l2 += w;
  root->s1 += v;
  root->s2 += w;
}

func query(root: dynamic)
{
  if ((root == null))
  {
    return;
  }
  ans[root->id] = root->s2;
  root->pushdown();
  if ((root->l != null))
  {
    query(root->l);
  }
  if ((root->r != null))
  {
    query(root->r);
  }
}

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(s[i].p);
      read(s[i].q);
      i += 1;
    }
  }
  sort((s + 1), ((s + n) + 1));
  read(m);
  var root = null;
  var k: dynamic;
  {
    var i = 1;
    while ((i <= m))
    {
      read(k);
      root = insert(root, cpp_new(k, i));
      i += 1;
    }
  }
  var l: dynamic;
  var r: dynamic;
  var x: dynamic;
  var y: dynamic;
  var L: dynamic;
  var R: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      split(root, l, r, s[i].p);
      if ((r == null))
      {
        root = l;
        i += 1;
        continue;
      }
      updata(r, (-s[i].p), 1);
      if ((l == null))
      {
        root = r;
        i += 1;
        continue;
      }
      x = left(r);
      y = right(l);
      while ((x->s1 < y->s1))
      {
        split(r, L, R, (x->s1 + 1));
        l = insert(l, L);
        r = R;
        if (((l == null) || (r == null)))
        {
          break;
        }
        x = left(r);
        y = right(l);
      }
      root = merge(l, r);
      i += 1;
    }
  }
  query(root);
  {
    var i = 1;
    while ((i <= m))
    {
      write(ans[i]);
      putchar(cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
