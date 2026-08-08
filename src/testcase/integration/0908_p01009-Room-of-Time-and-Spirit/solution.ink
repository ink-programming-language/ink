// Translated from solution.cpp.

func FOR(k: dynamic, m: dynamic, n: dynamic)
{
  cpp_macro("for(int (k)=(m);(k)<(n);(k)++)");
}

var LL = dynamic;

func CLR(a: dynamic)
{
  return cpp_expression("#include<iostream> #inc");
}

func SZ(x: dynamic)
{
  return cpp_expression("#include<iostream");
}

func WAITING(str: dynamic)
{
  cpp_macro("int str;std::cin>>str;");
}

func DEBUGING(str: dynamic)
{
  return cpp_expression("#include<iostre");
}

var MOD = 1000000007;

var INF = ((1 << 30));

func REP(i: dynamic, a: dynamic, n: dynamic)
{
  cpp_macro("for(ll i = ((ll) a); i < ((ll) n); i++)");
}

class UnionFind
{
  var parent: dynamic;
  var gap: dynamic;
  var up: dynamic;
  func UnionFind(n: dynamic)
  {
      this->parent = cpp_construct(n);
      this->gap = cpp_construct(n, 0);
      this->up = cpp_construct(n, 0);
      REP(i, 0, n)[i] = i;
    }
  func find(i: dynamic)
  {
      if ((parent[i] == i))
      {
        return i;
      }
      var tmp = parent[i];
      var p = find(parent[i]);
      gap[i] += gap[tmp];
      return cpp_assign(parent[i], "=", p);
    }
  func unite(i: dynamic, j: dynamic, c: dynamic)
  {
      var p = find(i);
      var q = find(j);
      if ((p != q))
      {
        gap[p] = ((c + get(j)) - get(i));
        parent[p] = q;
      }
      up[i] += c;
      up[j] += c;
    }
  func get(i: dynamic)
  {
      return ((-up[i]) + gap[i]);
    }
}

func main(argument_0: dynamic)
{
  var N: dynamic;
  var Q: dynamic;
  read(N, Q);
  REP(i, 0, Q);
  {
    var type_cpp: dynamic;
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
  var n: dynamic;
  read(n);
}
