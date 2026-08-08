// Translated from solution.cpp.

var ll = dynamic;

var PI = cpp_expression("#include <cstdio> #include <algor");

func read(x: dynamic)
{
  cpp_macro("scanf(\"%d\",&x);");
}

func readll(x: dynamic)
{
  cpp_macro("cin>>x;");
}

func FOR(x: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int x=a;x<b;x++)");
}

var MP = cpp_expression("#include");

var PB = cpp_expression("#include");

var pii = cpp_expression("#include <cst");

func readN(N: dynamic, X: dynamic)
{
  cpp_macro("for(int i=0;i<N;i++) cin>>X[i];");
}

var pff = cpp_expression("#include <cstdio> #");

var N: dynamic;

var M: dynamic;

var a = cpp_array(200005);

var pq: dynamic;

func cmp(A: dynamic, B: dynamic)
{
  if ((A.first == B.first))
  {
    return ((A.second > B.second));
  }
  return (A.first < B.first);
}

func main()
{
  ios.sync_with_stdio(false);
  read(N, M);
  FOR(i, 0, N);
  {
    read(a[i].first, a[i].second);
  }
  sort(a, (a + N), cmp);
  var j = 0;
  var ans = 0;
  {
    var i = 1;
    while ((i <= M))
    {
      while (((j < N) && (a[j].first < i)))
      {
        pq.push(a[cpp_update(j, "++")].second);
      }
      if (((j < N) && (a[j].first >= i)))
      {
        j += 1;
        i += 1;
        continue;
      }
      if ((pq.size() && (pq.top() <= i)))
      {
        pq.pop();
      }
      i += 1;
    }
  }
  while ((j < N))
  {
    j += 1;
    ans += 1;
  }
  write((ans + pq.size()), "\n");
}
