// Translated from solution.cpp.

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  var A: dynamic;
  var B: dynamic;
  read(n, A, B);
  {
    int_cpp(i) = 0;
    while (((i) < (n)))
    {
      read(v[i]);
      i += 1;
    }
  }
  var sum = accumulate(v.begin(), v.end(), 0);
  var flow = ((A * v[0]) / sum);
  var ans = 0;
  sort((v.begin() + 1), v.end());
  reverse((v.begin() + 1), v.end());
  var i = 1;
  while ((flow < B))
  {
    sum -= v[i];
    ans += 1;
    i += 1;
    flow = ((A * v[0]) / sum);
  }
  write(ans, cpp_char("\n"));
  return 0;
}
