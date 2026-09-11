// Translated from solution.cpp.

class Treap
{
  var key: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var lz: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func Treap() -> dynamic
  {
    }
  func Treap(key: dynamic) -> dynamic
  {
      self->key = key;
      self->p = rand();
      l = cpp_assign(r, "=", null);
      lz = 0;
    }
  func unlz() -> dynamic
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
  func size() -> dynamic
  {
      var sz: dynamic = 1;
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
  func getmin() -> dynamic
  {
      unlz();
      var res: dynamic = key;
      if (l)
      {
        res = l->getmin();
      }
      return res;
    }
}

var Root: dynamic = cpp_uninitialized();

func split(root: dynamic, k: dynamic, L: dynamic, R: dynamic) -> dynamic
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

func merge(A: dynamic, B: dynamic) -> dynamic
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
    return ( (A) ? A : B);
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

func insert(root: dynamic, item: dynamic) -> dynamic
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
    insert( ((item->key < root->key)) ? root->l : root->r, item);
  }
}

func erase(root: dynamic, k: dynamic) -> dynamic
{
  if ((!root))
  {
    return;
  }
  root->unlz();
  if ((root->key == k))
  {
    var newr: dynamic = merge(root->l, root->r);
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

func update(l: dynamic, r: dynamic) -> dynamic
{
  var L: dynamic = cpp_uninitialized();
  var mid: dynamic = cpp_uninitialized();
  var R: dynamic = cpp_uninitialized();
  var aux: dynamic = cpp_uninitialized();
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

var N: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  srand(time(0));
  insert(Root, cpp_new(0));
  read(N);
  while (cpp_update(N, "--"))
  {
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    read(l, r);
    update(l, r);
  }
  write((Root->size() - 1), "\n");
  return 0;
}
