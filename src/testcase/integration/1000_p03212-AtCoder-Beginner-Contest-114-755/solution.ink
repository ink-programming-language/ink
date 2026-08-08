// Translated from solution.cpp.

var N: dynamic;

var count = 0;

func dfs(nextnum: dynamic, flag: dynamic)
{
  if ((nextnum <= N))
  {
    if ((flag == 7))
    {
      count += 1;
    }
    dfs(((nextnum * 10) + 3), (flag | 01));
    dfs(((nextnum * 10) + 5), (flag | 02));
    dfs(((nextnum * 10) + 7), (flag | 04));
  }
}

func main()
{
  scanf("%d", (&N));
  dfs(0, 0);
  printf("%d", count);
  return 0;
}
