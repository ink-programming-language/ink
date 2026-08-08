// Translated from solution.cpp.

var MAXN = 2100;

var N: dynamic;

var S: dynamic;

var edge = cpp_array(MAXN);

var nnode = cpp_array(MAXN);

var ddep = cpp_array(MAXN);

var dmin = cpp_array(MAXN);

var ans: dynamic;

func flood(cloc: dynamic, last: dynamic)
{
  var ntot = 0;
  var nbest = 0;
  var nmin = 0;
  nnode[cloc] = (S[cloc] - cpp_char("0"));
  ddep[cloc] = 0;
  for (var neigh in edge[cloc])
  {
    if ((neigh == last))
    {
      continue;
    }
    flood(neigh, cloc);
    nnode[cloc] += nnode[neigh];
    var nb = (ddep[neigh] + nnode[neigh]);
    ddep[cloc] += nb;
    ntot += nb;
    if ((nb > nbest))
    {
      nbest = nb;
      nmin = (dmin[neigh] + nnode[neigh]);
    }
  }
  dmin[cloc] = max(0, ((nbest + nmin) - ntot));
}

func solve_root(x: dynamic)
{
  flood(x, -1);
  if (((dmin[x] == 0) && ((ddep[x] % 2) == 0)))
  {
    ans = min(ans, ddep[x]);
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  read(N, S);
  {
    var i = 0;
    while ((i < (N - 1)))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      a -= 1;
      b -= 1;
      edge[a].push_back(b);
      edge[b].push_back(a);
      i += 1;
    }
  }
  ans = 1e9;
  {
    var i = 0;
    while ((i < N))
    {
      solve_root(i);
      i += 1;
    }
  }
  if ((ans > 1e8))
  {
    write("-1\n");
  } else
  {
    write((ans / 2), "\n");
  }
}
