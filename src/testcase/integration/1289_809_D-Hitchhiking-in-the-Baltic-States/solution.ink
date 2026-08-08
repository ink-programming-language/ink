// Translated from solution.cpp.

class Treap
{
  var key: dynamic;
  var p: dynamic;
  var lz: dynamic;
  var l: dynamic;
  var r: dynamic;
  func Treap()
  {
    }
  func Treap(key: dynamic)
  {
      this->key = key;
      this->p = rand();
      l = cpp_assign(r, "=", null);
      lz = 0;
    }
  func unlz()
  {
      key += lz;
      if (l)
      {
        l->lz += lz;
      }
      if (r)
      {
        r->lz += lz;
      }
      lz = 0;
    }
  func size()
  {
      var sz = 1;
      if (l)
      {
        sz += l->size();
      }
      if (r)
      {
        sz += r->size();
      }
      return sz;
    }
  func getmin()
  {
      unlz();
      var res = key;
      if (l)
      {
        res = l->getmin();
      }
      return res;
    }
}

var Root: dynamic;

func split(root: dynamic, k: dynamic, L: dynamic, R: dynamic)
{
  if ((!root))
  {
    L = cpp_assign(R, "=", null);
    return;
  }
  root->unlz();
  if ((k >= root->key))
  {
    split(root->r, k, root->r, R);
    L = root;
  } else
  {
    split(root->l, k, L, root->l);
    R = root;
  }
}

func merge(A: dynamic, B: dynamic)
{
  if (A)
  {
    A->unlz();
  }
  if (B)
  {
    B->unlz();
  }
  if (((!A) || (!B)))
  {
    return (if (A) A else B);
  }
  if ((A->p > B->p))
  {
    A->r = merge(A->r, B);
    return A;
  } else
  {
    B->l = merge(A, B->l);
    return B;
  }
}

func insert(root: dynamic, item: dynamic)
{
  if ((!root))
  {
    root = item;
    return;
  }
  root->unlz();
  if ((item->p > root->p))
  {
    split(root, item->key, item->l, item->r);
    root = item;
  } else
  {
    insert(if ((item->key < root->key)) root->l else root->r, item);
  }
}

func erase(root: dynamic, k: dynamic)
{
  if ((!root))
  {
    return;
  }
  root->unlz();
  if ((root->key == k))
  {
    var newr = merge(root->l, root->r);
    cpp_delete(root);
    root = newr;
  } else if ((k < root->key))
  {
    erase(root->l, k);
  } else
  {
    erase(root->r, k);
  }
}

func update(l: dynamic, r: dynamic)
{
  var L: dynamic;
  var mid: dynamic;
  var R: dynamic;
  var aux: dynamic;
  split(Root, (l - 1), L, aux);
  split(aux, (r - 1), mid, R);
  if (R)
  {
    erase(R, R->getmin());
  }
  if (mid)
  {
    mid->lz += 1;
  }
  Root = merge(L, merge(mid, R));
  insert(Root, cpp_new(l));
}

var N: dynamic;

func main()
{
  srand(time(0));
  insert(Root, cpp_new(0));
  read(N);
  while (cpp_update(N, "--"))
  {
    var l: dynamic;
    var r: dynamic;
    read(l, r);
    update(l, r);
  }
  write((Root->size() - 1), "\n");
  return 0;
}
