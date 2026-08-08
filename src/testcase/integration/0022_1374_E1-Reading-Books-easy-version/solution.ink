// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var a: dynamic;

var b: dynamic;

var t: dynamic;

var ve = cpp_array(2, 2);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n, k);
  {
    var i = (0);
    while ((i < (n)))
    {
      read(t, a, b);
      ve[a][b].push_back(t);
      i += 1;
    }
  }
  sort((ve[0][1]).begin(), (ve[0][1]).end());
  sort((ve[1][0]).begin(), (ve[1][0]).end());
  var sums: dynamic;
  for (var x in ve[1][1])
  {
    sums.push_back(x);
  }
  {
    var i = 0;
    while ((i < min((cpp_cast((ve[0][1]).size())), (cpp_cast((ve[1][0]).size())))))
    {
      sums.push_back((ve[0][1][i] + ve[1][0][i]));
      i += 1;
    }
  }
  sort((sums).begin(), (sums).end());
  if (((cpp_cast((sums).size())) < k))
  {
    write("-1\n");
    return 0;
  }
  write(accumulate(sums.begin(), (sums.begin() + k), 0), cpp_char("\n"));
}
