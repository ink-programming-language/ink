// Translated from solution.cpp.

class Node
{
  var cnt: dynamic = cpp_uninitialized();
  var key: dynamic = cpp_uninitialized();
  var prior: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_array(5);
  var left: dynamic = cpp_uninitialized();
  var right: dynamic = cpp_uninitialized();
}

var nodes: dynamic = cpp_array(111111);

var pri: dynamic = cpp_array(111111);

var nodeCount: dynamic = 0;

func cnt(v: dynamic) -> dynamic
{
  return  (v) ? v->cnt : 0;
}

func update(v: dynamic) -> dynamic
{
  if (v)
  {
    v->cnt = ((cnt(v->left) + cnt(v->right)) + 1);
    var off: dynamic = 0;
    if (v->left)
    {
      {
        var i: dynamic = 0;
        while ((i < cpp_cast((5))))
        {
          v->sum[i] = v->left->sum[i];
          i += 1;
        }
      }
      off = (v->left->cnt % 5);
    } else
    {
      {
        var i: dynamic = 0;
        while ((i < cpp_cast((5))))
        {
          v->sum[i] = 0;
          i += 1;
        }
      }
    }
    v->sum[off] += v->key;
    if ((cpp_update(off, "++") == 5))
    {
      off = 0;
    }
    if (v->right)
    {
      {
        var i: dynamic = 0;
        while ((i < cpp_cast((5))))
        {
          var ii: dynamic = (i + off);
          if ((ii >= 5))
          {
            ii -= 5;
          }
          v->sum[ii] += v->right->sum[i];
          i += 1;
        }
      }
    }
  }
}

func merge(l: dynamic, r: dynamic, t: dynamic) -> dynamic
{
  if (((!l) || (!r)))
  {
    t =  (l) ? l : r;
    return;
  }
  if ((l->prior > r->prior))
  {
    merge(l->right, r, l->right);
    t = l;
  } else
  {
    merge(l, r->left, r->left);
    t = r;
  }
  update(t);
}

func split(t: dynamic, l: dynamic, r: dynamic, key: dynamic) -> dynamic
{
  if ((!t))
  {
    l = cpp_assign(r, "=", null);
    return;
  }
  if ((key < t->key))
  {
    split(t->left, l, t->left, key);
    r = t;
  } else
  {
    split(t->right, t->right, r, key);
    l = t;
  }
  update(t);
}

var root: dynamic = null;

func addKey(x: dynamic) -> dynamic
{
  var t1: dynamic = cpp_uninitialized();
  var t2: dynamic = cpp_uninitialized();
  var t3: dynamic = cpp_uninitialized();
  split(root, t1, t3, x);
  nodes[nodeCount].key = x;
  nodes[nodeCount].prior = pri[nodeCount];
  t2 = (nodes + cpp_update(nodeCount, "++"));
  update(t2);
  merge(t1, t2, root);
  merge(root, t3, root);
}

func delKey(t: dynamic, x: dynamic) -> dynamic
{
  if ((t->key == x))
  {
    merge(t->left, t->right, t);
  } else if ((x < t->key))
  {
    delKey(t->left, x);
  } else
  {
    delKey(t->right, x);
  }
  update(t);
}

var mt: dynamic = cpp_uninitialized();

func myRand(bound: dynamic) -> dynamic
{
  return (mt() % bound);
}

var s: dynamic = cpp_array(10);

var zzz: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((111111))))
    {
      pri[i] = i;
      i += 1;
    }
  }
  random_shuffle(pri, (pri + 111111), myRand);
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  {
    var query: dynamic = 0;
    while ((query < cpp_cast((q))))
    {
      scanf("%s", s);
      if ((s[0] == cpp_char("a")))
      {
        scanf("%d", (&zzz));
        addKey(zzz);
      } else if ((s[0] == cpp_char("d")))
      {
        scanf("%d", (&zzz));
        delKey(root, zzz);
      } else
      {
        if ((root == null))
        {
          printf("0\n");
        } else
        {
          printf("%I64d\n", root->sum[2]);
        }
      }
      query += 1;
    }
  }
  return 0;
}
