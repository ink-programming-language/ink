// Translated from solution.cpp.

func FOR(k: dynamic, m: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int (k)=(m);(k)<(n);(k)++)");
}

var LL: dynamic = dynamic;

func CLR(a: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream> #inc");
}

func SZ(x: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream");
}

func WAITING(str: dynamic) -> dynamic
{
  cpp_macro("int str;std::cin>>str;");
}

func DEBUGING(str: dynamic) -> dynamic
{
  return cpp_expression("#include<iostre");
}

var MOD: dynamic = 1000000007;

var INF: dynamic = ((1 << 30));

func REP(i: dynamic, a: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll i = ((ll) a); i < ((ll) n); i++)");
}

class UnionFind
{
  var parent: dynamic = cpp_uninitialized();
  var gap: dynamic = cpp_uninitialized();
  var up: dynamic = cpp_uninitialized();
  func UnionFind(n: dynamic) -> dynamic
  {
      self->parent = cpp_construct(n);
      self->gap = cpp_construct(n, 0);
      self->up = cpp_construct(n, 0);
      REP(i, 0, n)[i] = i;
    }
  func find(i: dynamic) -> dynamic
  {
      if ((parent[i] == i))
      {
        return i;
      }
      var tmp: dynamic = parent[i];
      var p: dynamic = find(parent[i]);
      gap[i] += gap[tmp];
      return cpp_assign(parent[i], "=", p);
    }
  func unite(i: dynamic, j: dynamic, c: dynamic) -> dynamic
  {
      var p: dynamic = find(i);
      var q: dynamic = find(j);
      if ((p != q))
      {
        gap[p] = ((c + get(j)) - get(i));
        parent[p] = q;
      }
      up[i] += c;
      up[j] += c;
    }
  func get(i: dynamic) -> dynamic
  {
      return ((-up[i]) + gap[i]);
    }
}

func main(argument_0: dynamic) -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_uninitialized();
  read(N, Q);
  REP(i, 0, Q);
  {
    var type_cpp: dynamic = cpp_uninitialized();
    read(type_cpp, A[i], B[i]);
    A[i] -= 1;
    B[i] -= 1;
    if ((type_cpp == "IN"))
    {
      T[i] = 0;
      read(C[i]);
    }
    if ((type_cpp == "COMPARE"))
    {
      T[i] = 1;
    }
  }
  REP(i, 0, Q);
  {
    if ((T[i] == 0))
    {
      uf.unite(A[i], B[i], C[i]);
    }
    if ((T[i] == 1))
    {
      if ((uf.find(A[i]) != uf.find(B[i])))
      {
        write("WARNING", "\n");
      } else
      {
        write((uf.get(A[i]) - uf.get(B[i])), "\n");
      }
    }
  }
  var n: dynamic = cpp_uninitialized();
  read(n);
}
