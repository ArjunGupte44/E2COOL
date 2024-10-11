import { MongoClient } from 'mongodb';

// Replace the following with your MongoDB connection string.
const uri = 'mongodb://localhost:27017';
const dbName = 'numbers';
const collectionName = 'num';

async function main(target: number) {
    // The goal of this typescript code is to find the closest number in a database from the given number. 
    // The database is numbers 1 to 100,000 in linear order 
    // The code does a linear search 
  const client = new MongoClient(uri);
  
  console.time('ExecutionTime'); // Start the timer

  try {
    // Connect to the MongoDB cluster
    await client.connect();
    const database = client.db(dbName);
    const collection = database.collection(collectionName);

    // Find the closest number to the target number
    // We sort by the absolute difference and get the first document

    let closest = null;
    let smallestDiff = Number.MAX_SAFE_INTEGER;

    // Cursor to iterate over all documents in the collection
    const cursor = collection.find();

    // Loop through all documents to find the closest number
    // The database is 1 to 100,000 in linear order
    for await (const doc of cursor) {
        const diff = Math.abs(target - doc.num);
        if (diff < smallestDiff) {
            smallestDiff = diff;
            closest = doc;
        }
    };

    if (closest) {
        console.log(`The closest number to ${target} is ${closest.num} with _id: ${closest._id}`);
    } else {
        console.log('No numbers found in the database.');
    }

  } finally {
      // Close the connection to the MongoDB cluster
      await client.close();

      console.timeEnd('ExecutionTime'); // End the timer and log the result
  }
}

main(59000).catch(console.error);