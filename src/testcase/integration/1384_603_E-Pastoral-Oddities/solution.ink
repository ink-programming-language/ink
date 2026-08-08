// Translated from solution.cpp.

var MAXN = 100000;

var MAXM = 300000;

class node
{
  var key: dynamic;
  var id: dynamic;
  var rev: dynamic;
  var siz1: dynamic;
  var siz2: dynamic;
  var s: dynamic;
  var fa: dynamic;
  var ch: dynamic;
  var mx: dynamic;
}

var tree = cpp_array(((MAXN + MAXM) + 5));

var ad: dynamic;

class edge
{
  var u: dynamic;
  var v: dynamic;
  var w: dynamic;
  var id: dynamic;
  func edge(u: dynamic = 0, v: dynamic = 0, w: dynamic = 0, i: dynamic = 0)
  {
      this->u = cpp_construct(u);
      this->v = cpp_construct(v);
      this->w = cpp_construct(w);
      this->id = cpp_construct(i);
    }
}

var e = cpp_array((MAXM + 5));

func operator_less(a: dynamic, b: dynamic)
{
  if ((a.w == b.w))
  {
    return (a.id < b.id);
  }
  return (a.w < b.w);
}

var Set: dynamic;

var it: dynamic;

var NIL: dynamic;

var ncnt: dynamic;

func Init()
{
  NIL = cpp_assign(ncnt, "=", (&tree[0]));
  NIL->fa = cpp_assign(NIL->ch[0], "=", cpp_assign(NIL->ch[1], "=", cpp_assign(NIL->mx, "=", NIL)));
  NIL->key = -1;
}

func IsRoot(x: dynamic)
{
  return (((x->fa == NIL)) || (((x->fa->ch[0] != x) && (x->fa->ch[1] != x))));
}

func SetChild(x: dynamic, y: dynamic, d: dynamic)
{
  x->ch[d] = y;
  if ((y != NIL))
  {
    y->fa = x;
  }
}

func NewNode(k: dynamic, id: dynamic, s: dynamic)
{
  ncnt += 1;
  ncnt->key = k;
  ncnt->id = id;
  ncnt->s = cpp_assign(ncnt->siz1, "=", s);
  ncnt->fa = cpp_assign(ncnt->ch[0], "=", cpp_assign(ncnt->ch[1], "=", NIL));
  ncnt->mx = ncnt;
  return ncnt;
}

func PushDown(x: dynamic)
{
  if (x->rev)
  {
    swap(x->ch[0], x->ch[1]);
    if ((x->ch[0] != NIL))
    {
      x->ch[0]->rev ^= 1;
    }
    if ((x->ch[1] != NIL))
    {
      x->ch[1]->rev ^= 1;
    }
    x->rev = 0;
  }
}

func PushUp(x: dynamic)
{
  if (((x->key > x->ch[0]->mx->key) && (x->key > x->ch[1]->mx->key)))
  {
    x->mx = x;
  } else if ((x->ch[0]->mx->key > x->ch[1]->mx->key))
  {
    x->mx = x->ch[0]->mx;
  } else
  {
    x->mx = x->ch[1]->mx;
  }
  x->siz1 = (((x->s + x->ch[0]->siz1) + x->ch[1]->siz1) + x->siz2);
}

func Rotate(x: dynamic)
{
  var y = x->fa;
  PushDown(y);
  PushDown(x);
  var d = ((y->ch[1] == x));
  if (IsRoot(y))
  {
    x->fa = y->fa;
  } else
  {
    SetChild(y->fa, x, (y->fa->ch[1] == y));
  }
  SetChild(y, x->ch[(!d)], d);
  SetChild(x, y, (!d));
  PushUp(y);
}

func Splay(x: dynamic)
{
  PushDown(x);
  while ((!IsRoot(x)))
  {
    var y = x->fa;
    if (IsRoot(y))
    {
      Rotate(x);
    } else
    {
      if ((((y->fa->ch[1] == y)) == ((y->ch[1] == x))))
      {
        Rotate(y);
      } else
      {
        Rotate(x);
      }
      Rotate(x);
    }
  }
  PushUp(x);
}

func Access(x: dynamic)
{
  var y = NIL;
  while ((x != NIL))
  {
    Splay(x);
    x->siz2 += x->ch[1]->siz1;
    SetChild(x, y, 1);
    x->siz2 -= x->ch[1]->siz1;
    PushUp(x);
    y = x;
    x = x->fa;
  }
}

func MakeRoot(x: dynamic)
{
  Access(x);
  Splay(x);
  x->rev ^= 1;
}

func Link(x: dynamic, y: dynamic)
{
  MakeRoot(x);
  MakeRoot(y);
  x->fa = y;
  y->siz2 += x->siz1;
}

func Cut(x: dynamic, y: dynamic)
{
  MakeRoot(x);
  Access(y);
  Splay(y);
  y->ch[0] = cpp_assign(x->fa, "=", NIL);
  PushUp(y);
}

func FindRoot(x: dynamic)
{
  Access(x);
  Splay(x);
  while ((x->ch[0] != NIL))
  {
    x = x->ch[0];
  }
  return x;
}

func QueryMAX(x: dynamic, y: dynamic)
{
  MakeRoot(x);
  Access(y);
  Splay(y);
  return y->mx;
}

var n: dynamic;

var m: dynamic;

var stot: dynamic;

func Debug()
{
  {
    it = Set.begin();
    while ((it != Set.end()))
    {
      printf("(%d, %d, %d)\n", it->u, it->v, it->w);
      it += 1;
    }
  }
}

func main()
{
  Init();
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      ad[i] = NewNode(-1, -1, 1);
      i += 1;
    }
  }
  stot = n;
  {
    var i = 1;
    while ((i <= m))
    {
      e[i].id = i;
      scanf("%d%d%d", (&e[i].u), (&e[i].v), (&e[i].w));
      if ((e[i].u == e[i].v))
      {
        i += 1;
        continue;
      }
      ad[(n + i)] = NewNode(e[i].w, i, 0);
      Set.insert(e[i]);
      if ((FindRoot(ad[e[i].u]) == FindRoot(ad[e[i].v])))
      {
        var p = QueryMAX(ad[e[i].u], ad[e[i].v]);
        Splay(p);
        if ((p->key > e[i].w))
        {
          Set.erase(e[p->id]);
          Cut(p, ad[e[p->id].u]);
          Cut(p, ad[e[p->id].v]);
          Link(ad[e[i].u], ad[(n + i)]);
          Link(ad[e[i].v], ad[(n + i)]);
        } else
        {
          Set.erase(e[i]);
        }
      } else
      {
        MakeRoot(ad[e[i].u]);
        MakeRoot(ad[e[i].v]);
        if ((((ad[e[i].u]->siz1 % 2) == 1) && ((ad[e[i].v]->siz1 % 2) == 1)))
        {
          stot -= 2;
        }
        Link(ad[e[i].u], ad[(n + i)]);
        Link(ad[e[i].v], ad[(n + i)]);
      }
      if ((stot == 0))
      {
        it = Set.end();
        it -= 1;
        while (true)
        {
          var p = ad[(n + it->id)];
          var q = ad[e[it->id].u];
          var r = ad[e[it->id].v];
          MakeRoot(p);
          Access(q);
          Access(r);
          if ((q->siz1 & 1))
          {
            break;
          }
          Access(q);
          if ((r->siz1 & 1))
          {
            break;
          }
          Set.erase(it);
          Cut(p, q);
          Cut(p, r);
          it = Set.end();
          it -= 1;
        }
        it = Set.end();
        it -= 1;
        printf("%d\n", it->w);
      } else
      {
        printf("-1\n");
      }
      i += 1;
    }
  }
}
