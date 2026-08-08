// Translated from solution.cpp.

class Tnode
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
}

var doing = cpp_array(1000050);

class Type
{
  var pos: dynamic;
  var val: dynamic;
  var size: dynamic;
  var father: dynamic;
  var son: dynamic;
  func Type()
  {
    }
  func Type(f: dynamic, p: dynamic, v: dynamic, s: dynamic)
  {
      pos = p;
      val = v;
      size = s;
      father = f;
      son[0] = cpp_assign(son[1], "=", 0);
    }
}

var memory = cpp_array(1000050);

var root: dynamic;

class Tree
{
  var best: dynamic;
  var add: dynamic;
}

var tree = cpp_array(2222222);

var vs: dynamic;

var n: dynamic;

var m: dynamic;

var data = cpp_array(1000050);

var x = cpp_array(1000050);

var y = cpp_array(1000050);

var z = cpp_array(1000050);

func Rand()
{
  return (((rand() << 15)) | rand());
}

func Cmp(a: dynamic, b: dynamic)
{
  return (a.x < b.x);
}

func Half(ask: dynamic)
{
  var low: dynamic;
  var mid: dynamic;
  var high: dynamic;
  low = 0;
  high = (m + 1);
  while (((low + 1) < high))
  {
    mid = (((low + high)) >> 1);
    if ((data[mid] <= ask))
    {
      low = mid;
    } else
    {
      high = mid;
    }
  }
  return low;
}

func Down(root: dynamic)
{
  if ((tree[root].add != 0))
  {
    tree[root].best += tree[root].add;
    tree[(root << 1)].add += tree[root].add;
    tree[(((root << 1)) | 1)].add += tree[root].add;
    tree[root].add = 0;
  }
  return;
}

func Add(root: dynamic, nowleft: dynamic, nowright: dynamic, askleft: dynamic, askright: dynamic, add: dynamic)
{
  var mid = (((nowleft + nowright)) >> 1);
  Down(root);
  if (((nowright < askleft) || (askright < nowleft)))
  {
    return;
  }
  if (((askleft <= nowleft) && (nowright <= askright)))
  {
    tree[root].add += add;
    Down(root);
    return;
  }
  Add((root << 1), nowleft, mid, askleft, askright, add);
  Add((((root << 1)) | 1), (mid + 1), nowright, askleft, askright, add);
  tree[root].best = min(tree[(root << 1)].best, tree[(((root << 1)) | 1)].best);
  return;
}

func Ask(root: dynamic, nowleft: dynamic, nowright: dynamic, askleft: dynamic, askright: dynamic)
{
  var mid = (((nowleft + nowright)) >> 1);
  Down(root);
  if (((nowright < askleft) || (askright < nowleft)))
  {
    return 666666;
  }
  if (((askleft <= nowleft) && (nowright <= askright)))
  {
    return tree[root].best;
  }
  return min(Ask((root << 1), nowleft, mid, askleft, askright), Ask((((root << 1)) | 1), (mid + 1), nowright, askleft, askright));
}

func Update(current: dynamic)
{
  if ((!current))
  {
    return;
  }
  current->size = 1;
  if (current->son[0])
  {
    current->size += current->son[0]->size;
  }
  if (current->son[1])
  {
    current->size += current->son[1]->size;
  }
  return;
}

func Rotate(current: dynamic, flag: dynamic)
{
  current->father->son[(flag ^ 1)] = current->son[flag];
  if (current->son[flag])
  {
    current->son[flag]->father = current->father;
  }
  current->son[flag] = current->father;
  if (current->father->father)
  {
    current->father->father->son[(current->father->father->son[0] != current->father)] = current;
  }
  current->father = current->father->father;
  current->son[flag]->father = current;
  Update(current->son[flag]);
  return;
}

func Splay(current: dynamic, target: dynamic)
{
  while ((current->father != target))
  {
    if ((current->father->father == target))
    {
      Rotate(current, (current->father->son[1] != current));
    } else if (((current->father->father->son[0] == current->father) && (current->father->son[0] == current)))
    {
      Rotate(current->father, 1);
      Rotate(current, 1);
    } else if (((current->father->father->son[1] == current->father) && (current->father->son[1] == current)))
    {
      Rotate(current->father, 0);
      Rotate(current, 0);
    } else
    {
      Rotate(current, (current->father->son[1] != current));
      Rotate(current, (current->father->son[1] != current));
    }
  }
  Update(current);
  if ((!target))
  {
    root = current;
  }
  return;
}

func Bigger(ask: dynamic, target: dynamic)
{
  var current = root;
  var best = 0;
  while (current)
  {
    if ((current->pos > ask))
    {
      best = current;
      current = current->son[0];
    } else
    {
      current = current->son[1];
    }
  }
  Splay(best, target);
  return;
}

func Smaller(ask: dynamic, target: dynamic)
{
  var current = root;
  var best = 0;
  while (current)
  {
    if ((current->pos < ask))
    {
      best = current;
      current = current->son[1];
    } else
    {
      current = current->son[0];
    }
  }
  Splay(best, target);
  return;
}

func Find(ask: dynamic, target: dynamic)
{
  var current = root;
  while (current)
  {
    if (current->son[0])
    {
      if ((ask == (current->son[0]->size + 1)))
      {
        break;
      }
    } else if ((ask == 1))
    {
      break;
    }
    if ((current->son[0] && (ask < (current->son[0]->size + 1))))
    {
      current = current->son[0];
    } else
    {
      ask -= 1;
      if (current->son[0])
      {
        ask -= current->son[0]->size;
      }
      current = current->son[1];
    }
  }
  Splay(current, target);
  return;
}

func Find_Left(current: dynamic)
{
  while (true)
  {
    if ((!current->son[0]))
    {
      break;
    }
    current = current->son[0];
  }
  Splay(current, 0);
  return;
}

func Find_Right(current: dynamic)
{
  while (true)
  {
    if ((!current->son[1]))
    {
      break;
    }
    current = current->son[1];
  }
  Splay(current, 0);
  return;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var temp: dynamic;
  var best: dynamic;
  var delta: dynamic;
  var rank: dynamic;
  var last: dynamic;
  var maxi: dynamic;
  var mx: dynamic;
  var my: dynamic;
  var mz: dynamic;
  var solved: dynamic;
  var current: dynamic;
  srand(cpp_cast(time(0)));
  scanf("%d", (&n));
  m = 0;
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&x[i]));
      data[cpp_update(m, "++")] = x[i];
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&y[i]));
      data[cpp_update(m, "++")] = y[i];
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&z[i]));
      data[cpp_update(m, "++")] = z[i];
      i += 1;
    }
  }
  sort((data + 1), ((data + m) + 1));
  {
    i = cpp_assign(j, "=", 1);
    while ((i < m))
    {
      if ((data[(i + 1)] != data[j]))
      {
        data[cpp_update(j, "++")] = data[(i + 1)];
      }
      i += 1;
    }
  }
  m = j;
  {
    i = 1;
    while ((i <= m))
    {
      doing[i].x = cpp_assign(doing[i].y, "=", cpp_assign(doing[i].z, "=", (n + 1)));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      temp = Half(x[i]);
      doing[temp].x = min(doing[temp].x, i);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      temp = Half(y[i]);
      doing[temp].y = min(doing[temp].y, i);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      temp = Half(z[i]);
      doing[temp].z = min(doing[temp].z, i);
      i += 1;
    }
  }
  sort((doing + 1), ((doing + m) + 1), Cmp);
  {
    i = 1;
    while ((i <= n))
    {
      Add(1, 1, n, i, i, i);
      i += 1;
    }
  }
  memory[cpp_update(vs, "++")] = Type(0, (n + 1), 0, 3);
  root = (&memory[vs]);
  memory[cpp_update(vs, "++")] = Type(root, -666666, 0, 1);
  root->son[0] = (&memory[vs]);
  memory[cpp_update(vs, "++")] = Type(root, 666666, 0, 1);
  root->son[1] = (&memory[vs]);
  best = (3 * n);
  mx = cpp_assign(my, "=", cpp_assign(mz, "=", 0));
  {
    i = 1;
    while ((i <= m))
    {
      mx = max(mx, doing[i].x);
      my = max(my, doing[i].y);
      mz = max(mz, doing[i].z);
      i += 1;
    }
  }
  if ((mx <= n))
  {
    best = min(best, mx);
  }
  if ((my <= n))
  {
    best = min(best, my);
  }
  if ((mz <= n))
  {
    best = min(best, mz);
  }
  maxi = 0;
  {
    i = m;
    while ((i >= 1))
    {
      solved = false;
      Smaller(doing[i].y, 0);
      Bigger(doing[i].y, root);
      if ((doing[i].z > root->son[1]->val))
      {
        if ((!root->son[1]->son[0]))
        {
          memory[cpp_update(vs, "++")] = Type(root->son[1], doing[i].y, doing[i].z, 1);
          current = cpp_assign(root->son[1]->son[0], "=", (&memory[vs]));
          Update(root->son[1]);
          Update(root);
        } else
        {
          solved = true;
          if ((root->son[1]->son[0]->val < doing[i].z))
          {
            current = root->son[1]->son[0];
            Splay(current, 0);
            delta = doing[i].z;
            if ((doing[i].z == (n + 1)))
            {
              delta = 666666;
            }
            delta -= current->val;
            if ((current->son[0]->size == 1))
            {
              Add(1, 1, n, 1, n, delta);
            } else
            {
              Find_Right(current->son[0]);
              Add(1, 1, n, root->pos, n, delta);
            }
            if ((doing[i].z <= n))
            {
              current->val = doing[i].z;
            } else
            {
              current->val = 666666;
            }
          }
        }
      } else
      {
        current = 0;
        solved = true;
      }
      if (current)
      {
        Splay(current, 0);
        if ((current->val == (n + 1)))
        {
          current->val = 666666;
        }
        if ((!solved))
        {
          delta = current->val;
          if ((current->son[0]->size == 1))
          {
            Find_Left(current->son[1]);
            delta -= root->val;
            if ((root->val < current->val))
            {
              Add(1, 1, n, 1, (current->pos - 1), delta);
            }
          } else
          {
            Find_Left(current->son[1]);
            delta -= root->val;
            if ((root->val < current->val))
            {
              Splay(current, 0);
              Find_Right(current->son[0]);
              Add(1, 1, n, root->pos, (current->pos - 1), delta);
            }
          }
        }
        while (true)
        {
          Splay(current, 0);
          rank = (current->son[0]->size + 1);
          if ((rank == 2))
          {
            break;
          }
          Find((rank - 1), 0);
          if ((root->val > current->val))
          {
            break;
          }
          delta = (current->val - root->val);
          last = (root->pos - 1);
          if ((rank == 3))
          {
            Add(1, 1, n, 1, last, delta);
          } else
          {
            Find((rank - 2), 0);
            Add(1, 1, n, root->pos, last, delta);
          }
          Find((rank - 2), 0);
          Find(rank, root);
          root->son[1]->son[0] = 0;
          Update(root->son[1]);
          Update(root);
        }
      }
      maxi = max(maxi, doing[i].z);
      if ((doing[(i - 1)].x <= n))
      {
        best = min(best, (doing[(i - 1)].x + Ask(1, 1, n, 1, n)));
        if ((maxi <= n))
        {
          best = min(best, (doing[(i - 1)].x + maxi));
        }
      }
      i -= 1;
    }
  }
  printf("%d\n", best);
  return 0;
}
